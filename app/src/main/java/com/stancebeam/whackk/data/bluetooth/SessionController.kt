package com.stancebeam.cc_lane.data.bluetooth

import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattDescriptor
import android.util.Log
import com.stancebeam.cc_lane.util.timestamp
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.TimeoutCancellationException
import kotlinx.coroutines.channels.awaitClose
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.callbackFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.withContext
import kotlinx.coroutines.withTimeout
import java.util.concurrent.ConcurrentHashMap
import javax.inject.Inject
import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException
import kotlin.coroutines.suspendCoroutine

private const val TAG = "SessionController"

enum class SessionState {
    IDLE,
    STARTING,
    ACTIVE,
    STOPPING,
    ERROR
}

class SessionController @Inject constructor(
    private val bluetoothController: BluetoothController,
) {

    companion object {
        private const val SESSION_TIMEOUT = 15000L
        private const val OPERATION_TIMEOUT = 5000L
        private const val OPERATION_DELAY = 100L
        private const val MAX_RETRIES = 3
        private const val PACKET_THRESHOLD_FA = 130
        private const val PACKET_THRESHOLD_FB = 130
    }

    private val connections get() = bluetoothController.connections

    // Session state management
    private val _sessionStates = MutableStateFlow<Map<String, SessionState>>(emptyMap())
    val sessionStates: StateFlow<Map<String, SessionState>> = _sessionStates.asStateFlow()

    private val sessionCallbacks = ConcurrentHashMap<String, SessionCallback>()
    private val retryAttempts = ConcurrentHashMap<String, Int>()

    @OptIn(ExperimentalStdlibApi::class)
    val swingFlow = callbackFlow {
        val faLists = ConcurrentHashMap<String, MutableList<String>>()
        val fbLists = ConcurrentHashMap<String, MutableList<String>>()
        val startTimestamps = ConcurrentHashMap<String, Long>()

        bluetoothController.setSessionCallback(object : SessionCallback {
            override fun onCharacteristicChanged(
                gatt: BluetoothGatt,
                characteristic: BluetoothGattCharacteristic,
                value: ByteArray,
            ) {
                val address = gatt.device.address

                // Only process data if session is active
                if (getSessionState(address) != SessionState.ACTIVE) {
                    Log.w(TAG, "Received data for inactive session: $address")
                    return
                }

                val faData = faLists.getOrPut(address, ::mutableListOf)
                val fbData = fbLists.getOrPut(address, ::mutableListOf)

                if (faData.isEmpty() && fbData.isEmpty()) {
                    startTimestamps[address] = timestamp()
                    Log.d(TAG, "Started data collection for $address")
                }

                val data = value.toHexString()
                Log.v(TAG, "onCharacteristicChanged: $address = $data")

                when {
                    data.startsWith("fa") -> faData.add(data)
                    data.startsWith("fb") -> fbData.add(data)
                    else -> Log.w(TAG, "Unknown data format: $data")
                }

                if (faData.size >= PACKET_THRESHOLD_FA && fbData.size >= PACKET_THRESHOLD_FB) {
                    val swingData = SwingData(
                        address = address,
                        startTs = startTimestamps[address] ?: 0L,
                        faPackets = faData.toList(),
                        fbPackets = fbData.toList(),
                    )

                    Log.d(TAG, "Swing data complete for $address: FA=${faData.size}, FB=${fbData.size}")
                    trySend(swingData)

                    // Clear data for next swing
                    faData.clear()
                    fbData.clear()
                    startTimestamps.remove(address)
                }
            }

            override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
                // Handle unexpected disconnections during session
                if (newState == BluetoothGatt.STATE_DISCONNECTED) {
                    val address = gatt.device?.address
                    if (address != null && getSessionState(address) == SessionState.ACTIVE) {
                        Log.w(TAG, "Device disconnected during active session: $address")
                        updateSessionState(address, SessionState.ERROR)
                        cleanupSession(address)
                    }
                }
            }
        })

        awaitClose {
            Log.d(TAG, "Swing flow closed")
            channel.close()
        }
    }

    suspend fun startSession(address: String): Result<Unit> = runCatching {
        withTimeout(SESSION_TIMEOUT) {
            Log.d("Session start->","Reaced Session Controller")
            startSessionInternal(address)
        }
    }.onFailure { e ->
        Log.e(TAG, "Failed to start session for $address", e)
        updateSessionState(address, SessionState.ERROR)
        when (e) {
            is TimeoutCancellationException -> {
                Log.e(TAG, "Session start timeout for $address")
            }
        }
    }

    private suspend fun startSessionInternal(address: String) {
        // Check if session is already starting or active
        val currentState = getSessionState(address)
        if (currentState == SessionState.STARTING || currentState == SessionState.ACTIVE) {
            Log.w(TAG, "Session already $currentState for $address")
            return
        }

        updateSessionState(address, SessionState.STARTING)
        BleDebugger.logSessionState(address, "STARTING")

        // Wait for device to be ready
        if (!bluetoothController.waitForDeviceReady(address, 10000L)) {
            throw Exception("Device not ready: $address")
        }

        val dataFormat = listOf(187, 1, 11, 100, 100, 100, 3, 3).map { it.toByte() }.toByteArray()

        var attempt = 0
        while (attempt < MAX_RETRIES) {
            try {
                // Use queued operations for better reliability
                Log.d(TAG, "Writing data format (attempt ${attempt + 1}): $address")
                val writeSuccess = bluetoothController.writeDataFormatQueued(address, dataFormat)
                if (!writeSuccess) {
                    throw Exception("Failed to write data format")
                }

                // Small delay between operations
                delay(OPERATION_DELAY)

                Log.d(TAG, "Enabling notifications (attempt ${attempt + 1}): $address")
                val notifySuccess = bluetoothController.setDataNotificationQueued(address, true)
                if (!notifySuccess) {
                    throw Exception("Failed to enable notifications")
                }

                updateSessionState(address, SessionState.ACTIVE)
                BleDebugger.logSessionState(address, "ACTIVE")
                retryAttempts.remove(address)
                return

            } catch (e: Exception) {
                attempt++
                Log.w(TAG, "Session start attempt $attempt failed for $address: ${e.message}")

                if (attempt >= MAX_RETRIES) {
                    throw Exception("Failed to start session after $MAX_RETRIES attempts: ${e.message}")
                }

                // Exponential backoff delay
                val backoffDelay = OPERATION_DELAY * (1 shl attempt)
                delay(backoffDelay)
            }
        }
    }

    suspend fun endSession(address: String): Result<Unit> = runCatching {
        withTimeout(SESSION_TIMEOUT) {
            endSessionInternal(address)
        }
    }.onFailure { e ->
        Log.e(TAG, "Failed to end session for $address", e)
        cleanupSession(address)
        when (e) {
            is TimeoutCancellationException -> {
                Log.e(TAG, "Session end timeout for $address")
            }
        }
    }

    private suspend fun endSessionInternal(address: String) {
        val currentState = getSessionState(address)
        if (currentState != SessionState.ACTIVE) {
            Log.w(TAG, "Cannot end session - current state: $currentState for $address")
            return
        }

        updateSessionState(address, SessionState.STOPPING)
        BleDebugger.logSessionState(address, "STOPPING")

        val dataFormat = listOf(187, 0, 11, 100, 100, 100, 3, 3).map { it.toByte() }.toByteArray()

        var attempt = 0
        while (attempt < MAX_RETRIES) {
            try {
                // First disable notifications
                Log.d(TAG, "Disabling notifications (attempt ${attempt + 1}): $address")
                val notifySuccess = bluetoothController.setDataNotificationQueued(address, false)
                if (!notifySuccess) {
                    throw Exception("Failed to disable notifications")
                }

                delay(OPERATION_DELAY)

                // Then write stop data format
                Log.d(TAG, "Writing stop data format (attempt ${attempt + 1}): $address")
                val writeSuccess = bluetoothController.writeDataFormatQueued(address, dataFormat)
                if (!writeSuccess) {
                    throw Exception("Failed to write stop data format")
                }

                updateSessionState(address, SessionState.IDLE)
                BleDebugger.logSessionState(address, "IDLE")
                cleanupSession(address)
                return

            } catch (e: Exception) {
                attempt++
                Log.w(TAG, "Session end attempt $attempt failed for $address: ${e.message}")

                if (attempt >= MAX_RETRIES) {
                    throw Exception("Failed to end session after $MAX_RETRIES attempts: ${e.message}")
                }

                val backoffDelay = OPERATION_DELAY * (1 shl attempt)
                delay(backoffDelay)
            }
        }
    }

    suspend fun slotEnd(): Result<Map<String, Result<Unit>>> = runCatching {
        Log.d(TAG, "slotEnd: Starting slot end for all active sessions")

        val activeAddresses = getActiveSessionAddresses()
        if (activeAddresses.isEmpty()) {
            Log.d(TAG, "slotEnd: No active sessions found")
            return@runCatching emptyMap<String, Result<Unit>>()
        }

        Log.d(TAG, "slotEnd: Found ${activeAddresses.size} active sessions: $activeAddresses")

        // End all active sessions and collect results
        val results = mutableMapOf<String, Result<Unit>>()

        // Process sessions sequentially to avoid overwhelming the BLE stack
        for (address in activeAddresses) {
            try {
                Log.d(TAG, "slotEnd: Ending session for $address")
                val result = endSessionInternal(address)
                results[address] = Result.success(result)

                // Small delay between operations
                delay(OPERATION_DELAY)

            } catch (e: Exception) {
                Log.e(TAG, "slotEnd: Failed to end session for $address", e)
                results[address] = Result.failure(e)
                // Force cleanup on failure
                cleanupSession(address)
                updateSessionState(address, SessionState.IDLE)
            }
        }

        Log.d(TAG, "slotEnd: Completed slot end. Results: ${results.mapValues { it.value.isSuccess }}")
        results

    }.onFailure { e ->
        Log.e(TAG, "slotEnd: Critical failure during slot end", e)
        // Emergency cleanup - force all sessions to IDLE
        getActiveSessionAddresses().forEach { address ->
            updateSessionState(address, SessionState.IDLE)
            cleanupSession(address)
        }
    }


    // Legacy methods with improved error handling
    suspend fun startSessionLegacy(address: String): Result<Unit> = runCatching {
        withTimeout(SESSION_TIMEOUT) {
            val gatt = connections[address] ?: throw Exception("Device not connected")

            if (!bluetoothController.waitForDeviceReady(address)) {
                throw Exception("Device not ready")
            }

            val dataFormat = listOf(187, 1, 11, 100, 100, 100, 3, 3).map { it.toByte() }.toByteArray()

            suspendCoroutine<Unit> { continuation ->
                var operationStep = 0 // 0: write, 1: notify

                bluetoothController.setSessionCallback(address, object : SessionCallback {
                    override fun onCharacteristicWrite(
                        gatt: BluetoothGatt,
                        characteristic: BluetoothGattCharacteristic,
                        status: Int
                    ) {
                        if (operationStep == 0) { // Write data format completed
                            if (status == BluetoothGatt.GATT_SUCCESS) {
                                operationStep = 1
                                bluetoothController.setDataNotification(gatt, true)
                            } else {
                                continuation.resumeWithException(
                                    Exception("Failed to write data format, status: $status")
                                )
                            }
                        }
                    }

                    override fun onDescriptorWrite(
                        gatt: BluetoothGatt,
                        descriptor: BluetoothGattDescriptor,
                        status: Int
                    ) {
                        if (operationStep == 1) { // Enable notification completed
                            if (status == BluetoothGatt.GATT_SUCCESS) {
                                updateSessionState(address, SessionState.ACTIVE)
                                continuation.resume(Unit)
                            } else {
                                continuation.resumeWithException(
                                    Exception("Failed to enable notifications, status: $status")
                                )
                            }
                        }
                    }
                })

                // Start the operation
                updateSessionState(address, SessionState.STARTING)
                bluetoothController.writeDataFormat(gatt, dataFormat)
            }
        }
    }.onFailure { e ->
        updateSessionState(address, SessionState.ERROR)
        when (e) {
            is TimeoutCancellationException -> {
                Log.e(TAG, "Legacy session start timeout for $address")
            }
            else -> {
                Log.e(TAG, "Legacy session start failed for $address", e)
            }
        }
    }

    suspend fun endSessionLegacy(address: String): Result<Unit> = runCatching {
        withTimeout(SESSION_TIMEOUT) {
            val gatt = connections[address] ?: throw Exception("Device not connected")
            val dataFormat = listOf(187, 0, 11, 100, 100, 100, 3, 3).map { it.toByte() }.toByteArray()

            suspendCoroutine<Unit> { continuation ->
                var operationStep = 0 // 0: disable notify, 1: write

                bluetoothController.setSessionCallback(address, object : SessionCallback {
                    override fun onDescriptorWrite(
                        gatt: BluetoothGatt,
                        descriptor: BluetoothGattDescriptor,
                        status: Int
                    ) {
                        if (operationStep == 0) { // Disable notification completed
                            if (status == BluetoothGatt.GATT_SUCCESS) {
                                operationStep = 1
                                bluetoothController.writeDataFormat(gatt, dataFormat)
                            } else {
                                continuation.resumeWithException(
                                    Exception("Failed to disable notifications, status: $status")
                                )
                            }
                        }
                    }

                    override fun onCharacteristicWrite(
                        gatt: BluetoothGatt,
                        characteristic: BluetoothGattCharacteristic,
                        status: Int
                    ) {
                        if (operationStep == 1) { // Write stop format completed
                            if (status == BluetoothGatt.GATT_SUCCESS) {
                                updateSessionState(address, SessionState.IDLE)
                                cleanupSession(address)
                                continuation.resume(Unit)
                            } else {
                                continuation.resumeWithException(
                                    Exception("Failed to write stop data format, status: $status")
                                )
                            }
                        }
                    }
                })

                // Start the operation
                updateSessionState(address, SessionState.STOPPING)
                bluetoothController.setDataNotification(gatt, false)
            }
        }
    }.onFailure { e ->
        cleanupSession(address)
        when (e) {
            is TimeoutCancellationException -> {
                Log.e(TAG, "Legacy session end timeout for $address")
            }
            else -> {
                Log.e(TAG, "Legacy session end failed for $address", e)
            }
        }
    }

    suspend fun writeCSV(faData: List<String>, fbData: List<String>): String {
        return withContext(Dispatchers.IO) {
            val csv = StringBuilder()
            csv.append("Index,FA_Data,FB_Data\n")

            val maxSize = maxOf(faData.size, fbData.size)
            for (i in 0 until maxSize) {
                val fa = faData.getOrNull(i) ?: ""
                val fb = fbData.getOrNull(i) ?: ""
                csv.append("$i,$fa,$fb\n")
            }

            csv.toString()
        }
    }

    // State management helpers
    private fun updateSessionState(address: String, state: SessionState) {
        _sessionStates.update { currentStates ->
            currentStates + (address to state)
        }
        Log.d(TAG, "Session state updated for $address: $state")
    }

    fun getSessionState(address: String): SessionState {
        return _sessionStates.value[address] ?: SessionState.IDLE
    }

    fun isSessionActive(address: String): Boolean {
        return getSessionState(address) == SessionState.ACTIVE
    }

    private fun cleanupSession(address: String) {
        sessionCallbacks.remove(address)
        retryAttempts.remove(address)
        bluetoothController.removeSessionCallback(address)
    }

    // Bulk operations
    suspend fun startMultipleSessions(addresses: List<String>): Map<String, Result<Unit>> {
        val results = mutableMapOf<String, Result<Unit>>()

        // Start sessions sequentially to avoid overwhelming the BLE stack
        for (address in addresses) {
            results[address] = startSession(address)
            delay(500) // Small delay between session starts
        }

        return results
    }

    suspend fun endMultipleSessions(addresses: List<String>): Map<String, Result<Unit>> {
        val results = mutableMapOf<String, Result<Unit>>()

        // End sessions in parallel since they're independent
        addresses.forEach { address ->
            results[address] = endSession(address)
        }

        return results
    }

    fun getActiveSessionAddresses(): List<String> {
        return _sessionStates.value.filterValues { it == SessionState.ACTIVE }.keys.toList()
    }

    fun release() {
        // End all active sessions
        val activeSessions = getActiveSessionAddresses()
        activeSessions.forEach { address ->
            updateSessionState(address, SessionState.IDLE)
            cleanupSession(address)
        }

        sessionCallbacks.clear()
        retryAttempts.clear()
        _sessionStates.value = emptyMap()
    }
}
