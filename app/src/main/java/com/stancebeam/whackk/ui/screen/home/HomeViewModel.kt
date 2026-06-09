package com.stancebeam.cc_lane.ui.screen.home

import android.Manifest
import android.content.Context
import android.content.Intent
import android.net.Uri
import android.util.Log
import androidx.annotation.RequiresPermission
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.documentfile.provider.DocumentFile
import androidx.lifecycle.LiveData
import androidx.lifecycle.SavedStateHandle
import androidx.lifecycle.ViewModel
import androidx.lifecycle.liveData
import androidx.lifecycle.viewModelScope
import com.stancebeam.cc_lane.data.JSBridge.JSBridgeController
import com.stancebeam.cc_lane.data.JSBridge.JSBridgeInterface
import com.stancebeam.cc_lane.data.JSBridge.JSBridgeMessage
import com.stancebeam.cc_lane.data.analytics.AnalyticsEngine
import com.stancebeam.cc_lane.data.analytics.DataConversion
import com.stancebeam.cc_lane.data.bluetooth.BluetoothController
import com.stancebeam.cc_lane.data.bluetooth.ConnectionController
import com.stancebeam.cc_lane.data.bluetooth.SessionController
import com.stancebeam.cc_lane.data.entity.BleSensor
import com.stancebeam.cc_lane.data.entity.Player
import com.stancebeam.cc_lane.data.entity.Session
import com.stancebeam.cc_lane.data.entity.Swing
import com.stancebeam.cc_lane.data.model.DeviceConnection
import com.stancebeam.cc_lane.data.model.demomodels.DeviceListItem
import com.stancebeam.cc_lane.data.model.demomodels.SensorData
import com.stancebeam.cc_lane.data.model.demomodels.SocketBle
import com.stancebeam.cc_lane.data.model.demomodels.SocketParameters
import com.stancebeam.cc_lane.data.network.WebSocketClient
import com.stancebeam.cc_lane.data.repository.BleSensorRepository
import com.stancebeam.cc_lane.data.repository.PlayerRepository
import com.stancebeam.cc_lane.data.repository.SessionRepository
import com.stancebeam.cc_lane.data.repository.SwingRepository
import com.stancebeam.cc_lane.data.util.Constants.LANE_APP_LANE_1
import com.stancebeam.cc_lane.data.util.Constants.MESSAGE_SENSOR
import com.stancebeam.cc_lane.data.util.Constants.MESSAGE_SESSION
import com.stancebeam.cc_lane.data.util.Constants.STATUS_INFO
import com.stancebeam.cc_lane.data.util.Constants.STATUS_INIT
import com.stancebeam.cc_lane.data.util.Constants.STATUS_SUCCESS
import com.stancebeam.cc_lane.data.util.Constants.SUB_TYPE_BALL_RELEASE
import com.stancebeam.cc_lane.data.util.Constants.SUB_TYPE_PAIR
import com.stancebeam.cc_lane.data.util.Constants.SUB_TYPE_PRE_RELEASE
import com.stancebeam.cc_lane.data.util.Constants.SUB_TYPE_SEARCH
import com.stancebeam.cc_lane.data.util.Constants.SUB_TYPE_START
import com.stancebeam.cc_lane.data.util.Constants.SUB_TYPE_SWING
import com.stancebeam.cc_lane.data.util.Constants.SUB_TYPE_UN_PAIR
import com.stancebeam.cc_lane.data.util.Constants.WEB_APP_LANE_1
import com.stancebeam.cc_lane.data.util.PlayerDefaults
import com.stancebeam.cc_lane.ui.screen.webview.WebViewState
import com.stancebeam.cc_lane.util.DEFAULT_SENSOR_NAME
import dagger.hilt.android.lifecycle.HiltViewModel
import dagger.hilt.android.qualifiers.ApplicationContext
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.collectLatest
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.flow.firstOrNull
import kotlinx.coroutines.launch
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import kotlinx.coroutines.withContext
import org.json.JSONObject
import java.io.BufferedReader
import java.io.File
import java.io.InputStreamReader
import javax.inject.Inject

@HiltViewModel
class HomeViewModel @Inject constructor(
    savedStateHandle: SavedStateHandle,
    private val webSocketClient: WebSocketClient,
    private val playerRepository: PlayerRepository,
    private val bluetoothController: BluetoothController,
    private val swingRepository: SwingRepository,
    private val sessionController: SessionController,
    private val sessionRepository: SessionRepository,
    connectionRepository: ConnectionController,
    private val jsBridgeController: JSBridgeController, // Inject JS Bridge Controller
    private  val jsBridgeInterface: JSBridgeInterface,
    @ApplicationContext private val context: Context,
    private val bleSensorRepository: BleSensorRepository
) : ViewModel() {

    private val sessionId: String = savedStateHandle["sessionId"] ?: "sessionID"

    private val _uiState = MutableStateFlow(WebViewState())
    val uiState = _uiState.asStateFlow()
    val scannedDevices = bluetoothController.scannedDevices
    val deviceConnections = connectionRepository.deviceConnections
    val swing = swingRepository.getSwingsAndPlayers(sessionId)
    val swingLatest = swingRepository.getSwings()

    var totalSwings by mutableIntStateOf(0)
        private set
    private var sessionDir by mutableStateOf<File?>(null)
    private val connections = mutableMapOf<String, DeviceConnection>()

    private var currentBall = 0
    private var totalBalls = 0
    private var sessionStarted = false

    private val TAG = "HomeViewModel"
    private val prefs = context.getSharedPreferences("lane_prefs", Context.MODE_PRIVATE)

    private var ballReleased by mutableStateOf(false)
    private var firstSwingAfterReleaseProcessed by mutableStateOf(false)
    private val ballReleaseLock = Mutex() // For thread safety


    // Holds the URL read from host.json
    private val _urlState = MutableStateFlow<String?>(null)
    val urlState: StateFlow<String?> = _urlState

    private val _folderUri = MutableStateFlow<Uri?>(null)
    val folderUri: StateFlow<Uri?> = _folderUri

    init {
        initializeFolder()

        //dummy player entry
        viewModelScope.launch {
            val playerData = Player(
                playerId = "123",
                fullName = "Anikesh",
                email = "abc@gmail.com"
            )
            playerRepository.deleteAll()
            playerRepository.insert(playerData)
        }

        // Replace WebSocket message handling with JS Bridge message handling
        viewModelScope.launch {
            jsBridgeController.messageFlow.collectLatest { message ->
                Log.d(TAG, "JS Bridge message: $message")
                handleJSBridgeMessage(message)
            }
        }

        // Scanned devices - sending to JS
        // In your init block, replace the scanned devices flow with this:
        viewModelScope.launch {
            scannedDevices.collectLatest { devices ->
                Log.d("BLEtest->", "scanned list ${devices.size}")
                if (devices.isNotEmpty()) {
                    val deviceListWithCustomNames = mutableListOf<DeviceListItem>()

                    devices.forEach { device ->
                        val macAddress = device.address
                        val defaultName = device.name ?: DEFAULT_SENSOR_NAME

                        // Check if custom name exists for this MAC address
                        val customName = getSensorDisplayName(macAddress, defaultName)

                        // Record the detection
                        onSensorDetected(macAddress, defaultName)

                        // Add to list with custom name
                        deviceListWithCustomNames.add(
                            DeviceListItem(customName, macAddress)
                        )
                    }

                    // Send the list with custom names to JS Bridge
                    jsBridgeController.sendDeviceList(deviceListWithCustomNames)
                }
            }
        }


        // Paired device - sending to JS
        viewModelScope.launch {
            deviceConnections.collectLatest { connectedDevices ->
                Log.d("BLEtest->", "connected list ${connectedDevices.size}")
                if (connectedDevices.isNotEmpty()) {
                    val device = connectedDevices[0].device
                    jsBridgeController.sendPairedDevice(
                        device.name ?: "",
                        device.address
                    )
                }
            }
        }

        // Swing data - sending to JS
        viewModelScope.launch {
            Log.d(TAG, "swingData: launch")
            swingLatest.collectLatest { swingData ->
                Log.d(TAG, "swingData: $swingData")

                if (swingData.isNotEmpty()) {
                    ballReleaseLock.withLock {
                        // Only process swing if ball has been released and first swing hasn't been processed yet
                        if (ballReleased && !firstSwingAfterReleaseProcessed) {
                            Log.d(TAG, "Processing first swing after ball release")

                            val lastSwing = swingData.last()
                            val sensorData = SensorData(
                                power = lastSwing.power,
                                powerAtImpact = lastSwing.powerAtImpact,
                                pSpeed = lastSwing.speed,
                                pSpeedAtImpact = lastSwing.speedAtImpact,
                                pEfficiency = lastSwing.efficiency,
                                pBackLift = lastSwing.backLift,
                                pDownSwing = lastSwing.downswing,
                                pFollowThrough = lastSwing.followThrough,
                                timeToImpact = lastSwing.timeToImpact,
                                batFace = lastSwing.batFace,
                                pShotType = lastSwing.shotType,
                            )

                            val socketBle = SocketBle(
                                pType = MESSAGE_SENSOR,
                                pSubType = SUB_TYPE_SWING,
                                pSender = LANE_APP_LANE_1,
                                pRecipient = WEB_APP_LANE_1,
                                pLaneId = 1,
                                pStatus = STATUS_INFO,
                                pParameters = SocketParameters(
                                    pCurrentBall = currentBall,
                                    pTotalBalls = totalBalls,
                                    pSensorData = SensorData(
                                        power = lastSwing.power,
                                        powerAtImpact = lastSwing.powerAtImpact,
                                        pSpeed = lastSwing.speed,
                                        pSpeedAtImpact = lastSwing.speedAtImpact,
                                        pEfficiency = lastSwing.efficiency,
                                        pBackLift = lastSwing.backLift,
                                        pDownSwing = lastSwing.downswing,
                                        pFollowThrough = lastSwing.followThrough,
                                        timeToImpact = lastSwing.timeToImpact,
                                        batFace = lastSwing.batFace,
                                        pShotType = lastSwing.shotType,
                                    )
                                )
                            )

                            webSocketClient.sendMessage2(socketBle)
                            jsBridgeController.sendSwingData(sensorData, currentBall, totalBalls)

                            // Mark first swing as processed
                            firstSwingAfterReleaseProcessed = true
                            Log.d(
                                TAG,
                                "First swing after ball release sent. Subsequent swings will be ignored."
                            )

                        } else if (ballReleased && firstSwingAfterReleaseProcessed) {
                            Log.d(
                                TAG,
                                "Ignoring swing data - first swing after ball release already processed"
                            )
                        } else if (!ballReleased) {
                            Log.d(TAG, "Ignoring swing data - no ball release detected yet")
                        }
                    }
                }
            }
        }
    }


    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    private fun handleJSBridgeMessage(message: JSBridgeMessage) {
        when (message) {
            is JSBridgeMessage.Search -> {
                handleSearch()
            }
            is JSBridgeMessage.Pair -> {
                handlePair(message.sensorAddress)
            }
            is JSBridgeMessage.UnPair -> {
                handleUnPair(message.sensorAddress)
            }
            is JSBridgeMessage.StartSession -> {
                handleStartSession(message.sessionId)
            }
            is JSBridgeMessage.PreRelease -> {
                currentBall = message.currentBall
                totalBalls = message.totalBalls
                // Reset ball release state for new ball preparation
                resetBallReleaseState()
                Log.d(TAG, "PreRelease: Ball $currentBall/$totalBalls - Reset ball release state")
            }
            is JSBridgeMessage.BallRelease -> {
                handleBallRelease()
            }
            is JSBridgeMessage.SessionComplete -> {
                // Handle session complete logic
                resetBallReleaseState()
                viewModelScope.launch {
                    sessionController.slotEnd()

                }

                sessionStarted = false

            }
            is JSBridgeMessage.SensorName -> {
                updateSensorName(macAddress = message.macId, newName = message.name)
            }
            is JSBridgeMessage.Logout ->{
                sessionController.release()
                 bluetoothController.disconnectAllDevices()
                sessionStarted = false
            }
            is JSBridgeMessage.Unknown -> {
                Log.w(TAG, "Unknown message: ${message.rawMessage}")
            }

        }
    }

    // New method to handle ball release
    private fun handleBallRelease() {
        viewModelScope.launch {
            ballReleaseLock.withLock {
                ballReleased = true
                firstSwingAfterReleaseProcessed = false
                Log.d(TAG, "Ball released - Ready to process first swing data")
            }
        }
    }

    // New method to reset ball release state
    private fun resetBallReleaseState() {
        viewModelScope.launch {
            ballReleaseLock.withLock {
                ballReleased = false
                firstSwingAfterReleaseProcessed = false
                Log.d(TAG, "Ball release state reset")

            }
        }
    }

    private fun handleSearch() {
        viewModelScope.launch {
//            try {
//                deviceConnections.firstOrNull()?.forEach { deviceData ->
//                    Log.d("BLEtest->", "disconnecting")
//                    bluetoothController.disconnectDevice(deviceData.device.address)
//                }
//            } catch (e: Exception) {
//                Log.e(TAG, "error on search", e)
//            }
            Log.d("BLEtest->", "scan started")
            bluetoothController.startScan()
        }
    }


    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    private fun handlePair(sensorAddress: String) {
        viewModelScope.launch {
            bluetoothController.stopScan()
            Log.d("BLEtest->", "PAIRED")
            bluetoothController.connectDevice(sensorAddress)
        }
    }

    private fun handleUnPair(sensorAddress: String) {
        viewModelScope.launch {
            try {
                deviceConnections.firstOrNull()?.forEach { deviceData ->
                    Log.d("BLEtest->", "unpairing")
                    sessionStarted = false
                    bluetoothController.disconnectDevice(deviceData.device.address)
                    delay(500)
                    jsBridgeController.sendUnPairedDevice(
                        deviceData.device.name.orEmpty(),
                        deviceData.device.address
                    )
                }
            } catch (e: Exception) {
                Log.e(TAG, "error on unpair", e)
            }
        }
    }

    private fun handleStartSession(sessionId: String) {
        if (!sessionStarted) {
            Log.d("BLETest->", "Session started ")
            sessionStarted = true
            startSession(sessionId)
            jsBridgeController.sendSessionStarted()
        }
    }
    fun saveFolderUri(uri: Uri) {
        viewModelScope.launch {
            try {
                // Take persistable permission
                context.contentResolver.takePersistableUriPermission(
                    uri,
                    Intent.FLAG_GRANT_READ_URI_PERMISSION or Intent.FLAG_GRANT_WRITE_URI_PERMISSION
                )

                // Update StateFlow
                _folderUri.value = uri

                // Save to SharedPreferences
                prefs.edit().putString("folder_uri", uri.toString()).apply()

                // Read host.json and update URL
                readHostJson(uri)
            } catch (e: Exception) {
                Log.e(TAG, "Error saving folder URI: ${e.message}")
            }
        }
    }

    /** Load persisted folder URI */
    fun loadFolderUri(): Uri? {
        val s = prefs.getString("folder_uri", null) ?: return null
        return try { Uri.parse(s) } catch (e: Exception) { null }
    }

    /** Read host.json from folder and update urlState */
    fun readHostJson(uri: Uri) {
        viewModelScope.launch(Dispatchers.IO) {
            try {
                val folderDoc = DocumentFile.fromTreeUri(context, uri)
                val file = folderDoc?.findFile("host.json")
                val json = file?.let {
                    context.contentResolver.openInputStream(it.uri)?.use { input ->
                        BufferedReader(InputStreamReader(input)).use { reader -> reader.readText() }
                    }
                }

                json?.let {
                    val obj = JSONObject(it)
                    val url = obj.optString("url", "")
                    if (url.isNotEmpty()) {
                        _urlState.value = url
                        Log.d(TAG, "Loaded URL from host.json: $url")
                    }
                }
            } catch (e: Exception) {
                Log.e(TAG, "Error reading host.json: ${e.message}")
            }
        }
    }

    /** Initialize folder and URL on app start */
    fun initializeFolder() {
        loadFolderUri()?.let { uri ->
            _folderUri.value = uri
            readHostJson(uri)
        }
    }

    // Expose the dependencies
    fun getJSBridgeController(): JSBridgeController = jsBridgeController
    fun getJSBridgeInterface(): JSBridgeInterface = jsBridgeInterface


    private fun startSession(sessionId: String) {


        //_uiState.value = UiState.Loading
        viewModelScope.launch {
            //val userId = checkNotNull(currentUser?.userId)
            val session = Session(
                userId = "userId",
            )
            val results = deviceConnections.first()
                .map { connection ->
                    val address = connection.device.address
                    try {
                        Log.d("Session start->","Triggered from viewmodel")
                        sessionController.startSession(address)
                        Log.d("HomeViewModel", "startSession: $address")
                        true
                    } catch (e: Exception) {
                        Log.d("HomeViewModel", "startSession: $address", e)
                        false
                    }
                }
            val success = results.any { it }
            if (success) {
                Log.d("HomeViewModel", "insert: if $session")
                sessionRepository.insert(session)
                Log.d("HomeViewModel", "insert: if")

                //UiState.Success(session.sessionId)
            } else {
                Log.d("HomeViewModel", "insert: else")
            }
        }

        viewModelScope.launch {
            sessionController.swingFlow.collectLatest {
                Log.d("HomeViewModel", "swingFlow")

                val faPackets = it.faPackets
                val fbPackets = it.fbPackets
                totalSwings++
                writeData(faPackets, fbPackets, totalSwings)
                val data = DataConversion.prepareDataForAE(faPackets, fbPackets)
                Log.d("HomeViewModel", "prepareDataForAE")

                //connections[it.address]?.player?.let { player ->
                AnalyticsEngine.initPlayer(
                    batWeight = 1.10,
                    batLength = 87.0,
                    gripPosition = PlayerDefaults.GripPosition.MEDIUM,//player.gripPosition,
                    orientationApp = 6.0,
                    orientationAna = 21.0,
                    userHand = PlayerDefaults.BattingHand.RIGHT,//player.battingHand,
                    firmwareVersion = 3,
                    gyroXThreshold = 500.0,
                    dpr = 100,
                )
                AnalyticsEngine.kinematics(
                    acc = data[0],
                    gyro = data[1],
                    angle = data[2],
                    mag = data[3],
                    magMaxMin = data[4],
                )
                val result = AnalyticsEngine.getResult()

                Log.d("HomeViewModel", "result")

                val swing = Swing(
                    playerId = "playerABC",//player.playerId,
                    sessionId = "sessionID",
                    swingNumber = totalSwings,
                    power = result.power,
                    powerAtImpact = result.powerAtImpact,
                    speed = result.speed,
                    speedAtImpact = result.speedAtImpact,
                    efficiency = result.efficiency,
                    backLift = result.backLift,
                    downswing = result.downswing,
                    followThrough = result.followThrough,
                    timeToImpact = result.timeToImpact,
                    batFace = result.batFace,
                    shotType = result.shotType,
                )
                Log.d("HomeViewModel", "swing")

                swingRepository.insert(swing)

                Log.d("HomeViewModel", "swing-insert")
                //}
            }
        }
    }


    fun search() {
        viewModelScope.launch {
            val socketBle = SocketBle(
                pType = MESSAGE_SENSOR,
                pSubType = SUB_TYPE_SEARCH,
                pSender = LANE_APP_LANE_1,
                pRecipient = WEB_APP_LANE_1,
                pLaneId = 1,
                pStatus = STATUS_INIT
            )
            webSocketClient.sendMessage2(socketBle)
        }
    }

    fun pair() {
        viewModelScope.launch {
            val socketBle = SocketBle(
                pType = MESSAGE_SENSOR,
                pSubType = SUB_TYPE_PAIR,
                pSender = LANE_APP_LANE_1,
                pRecipient = WEB_APP_LANE_1,
                pLaneId = 1,
                pStatus = STATUS_INIT,
                pParameters = SocketParameters(pSensorAddress = "E2:17:7B:28:92:07")
            )
            webSocketClient.sendMessage2(socketBle)
        }
    }

    fun startSession() {
        viewModelScope.launch {
            val socketBle = SocketBle(
                pType = MESSAGE_SESSION,
                pSubType = SUB_TYPE_START,
                pSender = LANE_APP_LANE_1,
                pRecipient = WEB_APP_LANE_1,
                pLaneId = 1,
                pStatus = STATUS_INIT,
            )
            webSocketClient.sendMessage2(socketBle)
        }
    }

    fun preRelease() {
        viewModelScope.launch {
            val socketBle = SocketBle(
                pType = MESSAGE_SESSION,
                pSubType = SUB_TYPE_PRE_RELEASE,
                pSender = LANE_APP_LANE_1,
                pRecipient = WEB_APP_LANE_1,
                pLaneId = 1,
                pStatus = STATUS_SUCCESS,
            )
            webSocketClient.sendMessage2(socketBle)
        }
    }

    fun ballRelease() {
        viewModelScope.launch {
            val socketBle = SocketBle(
                pType = MESSAGE_SESSION,
                pSubType = SUB_TYPE_BALL_RELEASE,
                pSender = LANE_APP_LANE_1,
                pRecipient = WEB_APP_LANE_1,
                pLaneId = 1,
                pStatus = STATUS_INFO,
            )
            webSocketClient.sendMessage2(socketBle)
        }
    }

    fun unPair() {
        viewModelScope.launch {
            val socketBle = SocketBle(
                pType = MESSAGE_SENSOR,
                pSubType = SUB_TYPE_UN_PAIR,
                pSender = LANE_APP_LANE_1,
                pRecipient = WEB_APP_LANE_1,
                pLaneId = 1,
                pStatus = STATUS_INIT,
                pParameters = SocketParameters(pSensorAddress = "E2:17:7B:28:92:07")
            )
            webSocketClient.sendMessage2(socketBle)
        }
    }

    private fun writeData(faPackets: List<String>, fbPackets: List<String>, swingNumber: Int) {
        if (sessionDir == null) return
        viewModelScope.launch {
            try {
                withContext(Dispatchers.IO) {
                    val filename = "Swing $swingNumber.csv"
                    val file = File(sessionDir, filename)
                    file.outputStream()
                        .apply {
                            write("FA,FB\n".toByteArray())
                            faPackets.forEachIndexed { index, item ->
                                write(item.toByteArray())
                                write(",".toByteArray())
                                write(fbPackets[index].toByteArray())
                                write("\n".toByteArray())
                            }
                            close()
                        }
                    Log.d("HomeViewModel", "writeData: $filename")
                }
            } catch (e: Exception) {
                Log.e("HomeViewModel", "writeData: ", e)
            }
        }
    }

    // Function to get sensor name when BLE device is detected
    suspend fun getSensorDisplayName(macAddress: String, defaultName: String = DEFAULT_SENSOR_NAME): String {
        return bleSensorRepository.getSensorNameOrDefault(macAddress, defaultName)
    }

    // Function to save/update sensor when detected
// Enhanced version of your onSensorDetected function
    fun onSensorDetected(macAddress: String, deviceName: String? = null) {
        viewModelScope.launch {
            try {
                val defaultName = deviceName ?: DEFAULT_SENSOR_NAME
                val existingSensor = bleSensorRepository.getSensorByMacAddress(macAddress)

                if (existingSensor != null) {
                    // Update last detected time
                    bleSensorRepository.updateLastDetectedTime(macAddress)
                    Log.d(TAG, "Updated detection time for sensor: $macAddress")
                } else {
                    // Save new sensor with default name
                    val newSensor = BleSensor(
                        macAddress = macAddress,
                        customName = defaultName,
                        lastDetectedTime = System.currentTimeMillis(),
                        isActive = true
                    )
                    bleSensorRepository.saveSensor(newSensor)
                    Log.d(TAG, "New sensor saved: $macAddress with name: $defaultName")
                }
            } catch (e: Exception) {
                Log.e(TAG, "Error handling sensor detection: ${e.message}")
            }
        }
    }


    // Function to update sensor name (for user customization)
    fun updateSensorName(macAddress: String, newName: String) {
        Log.d(TAG, "Updating sensor name: $macAddress -> $newName")
        viewModelScope.launch {
            try {
                // Update in database
                bleSensorRepository.updateSensorName(macAddress, newName)

                // Send confirmation to JS Bridge
                jsBridgeController.sendOnSensorNameChange(name = newName, macId = macAddress)

                Log.d(TAG, "Sensor name updated: $macAddress -> $newName")
            } catch (e: Exception) {
                Log.e(TAG, "Error updating sensor name: ${e.message}")
                // Optionally send error to JS Bridge
            }
        }
    }

    // Get all saved sensors
    fun getAllSavedSensors(): LiveData<List<BleSensor>> {
        return liveData {
            emit(bleSensorRepository.getAllActiveSensors())
        }
    }
    fun release(){
        sessionStarted = false
        bluetoothController.release()
        sessionController.release()
    }


}
