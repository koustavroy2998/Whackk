package com.stancebeam.cc_lane.data.JSBridge

import android.util.Log
import android.webkit.WebView
import com.stancebeam.cc_lane.data.model.demomodels.DeviceListItem
import com.stancebeam.cc_lane.data.model.demomodels.SensorData
import org.json.JSONObject
import org.json.JSONArray
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.SharedFlow
import kotlinx.coroutines.flow.asSharedFlow
import kotlinx.coroutines.launch
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class JSBridgeController @Inject constructor() {
    private val TAG = "JSBridgeController"
    private val scope = CoroutineScope(Dispatchers.Main)
    
    private var webView: WebView? = null
    
    // Message flow for communication with ViewModel
    private val _messageFlow = MutableSharedFlow<JSBridgeMessage>()
    val messageFlow: SharedFlow<JSBridgeMessage> = _messageFlow.asSharedFlow()

    fun setWebView(webView: WebView) {
        this.webView = webView
        Log.d(TAG, "WebView set for JS Bridge")
    }

    fun handleMessage(messageJson: String) {
        try {
            val jsonObject = JSONObject(messageJson)
            val message = parseMessage(jsonObject)
            scope.launch {
                _messageFlow.emit(message)
            }
        } catch (e: Exception) {
            Log.e(TAG, "Error parsing message: $messageJson", e)
        }
    }
    fun handleSensorNameChange(name: String,macId: String) {

        scope.launch {
            _messageFlow.emit(JSBridgeMessage.SensorName(name = name,macId=macId))
        }
    }

    fun handleSearch() {
        scope.launch {
            _messageFlow.emit(JSBridgeMessage.Search)
        }
    }

    fun handlePair(sensorAddress: String) {
        scope.launch {
            _messageFlow.emit(JSBridgeMessage.Pair(sensorAddress))
        }
    }

    fun handleUnPair(sensorAddress: String) {
        scope.launch {
            _messageFlow.emit(JSBridgeMessage.UnPair(sensorAddress))
        }
    }

    fun handleStartSession(sessionId: String) {
        scope.launch {
            _messageFlow.emit(JSBridgeMessage.StartSession(sessionId))
        }
    }

    fun handlePreRelease(currentBall: Int, totalBalls: Int) {
        scope.launch {
            _messageFlow.emit(JSBridgeMessage.PreRelease(currentBall, totalBalls))
        }
    }

    fun handleBallRelease() {
        scope.launch {
            _messageFlow.emit(JSBridgeMessage.BallRelease)
        }
    }

    fun handleSessionComplete() {
        scope.launch {
            _messageFlow.emit(JSBridgeMessage.SessionComplete)
        }
    }

    fun handleLogout() {
        scope.launch {
            _messageFlow.emit(JSBridgeMessage.Logout)
        }
    }




    // Send data to JavaScript
    fun sendToJS(functionName: String, data: String) {
        webView?.let { wv ->
            scope.launch {
                val js = "javascript:$functionName('$data')"
                Log.d(TAG, "Sending to JS: $js")
                wv.evaluateJavascript(js, null)
            }
        }
    }

    fun sendDeviceList(devices: List<DeviceListItem>) {
        val jsonArray = JSONArray()
        devices.forEach { device ->
            val deviceJson = JSONObject().apply {
                put("name", device.pSensorName)
                put("address", device.pSensorAddress)
            }
            jsonArray.put(deviceJson)
        }
        sendToJS("onDeviceListReceived", jsonArray.toString())
    }

    fun sendPairedDevice(name: String, address: String) {
        val deviceJson = JSONObject().apply {
            put("name", name)
            put("address", address)
        }
        sendToJS("onDevicePaired", deviceJson.toString())
    }

    fun sendUnPairedDevice(name: String, address: String) {
        val deviceJson = JSONObject().apply {
            put("name", name)
            put("address", address)
        }
        sendToJS("onDeviceUnPaired", deviceJson.toString())
    }

    fun sendSwingData(sensorData: SensorData, currentBall: Int, totalBalls: Int) {
        val swingJson = JSONObject().apply {
            put("currentBall", currentBall)
            put("totalBalls", totalBalls)
            put("sensorData", JSONObject().apply {
                put("power", sensorData.power)
                put("powerAtImpact", sensorData.powerAtImpact)
                put("speed", sensorData.pSpeed)
                put("speedAtImpact", sensorData.pSpeedAtImpact)
                put("efficiency", sensorData.pEfficiency)
                put("backLift", sensorData.pBackLift)
                put("downSwing", sensorData.pDownSwing)
                put("followThrough", sensorData.pFollowThrough)
                put("timeToImpact", sensorData.timeToImpact)
                put("batFace", sensorData.batFace)
                put("shotType", sensorData.pShotType)
            })
        }
        sendToJS("onSwingDataReceived", swingJson.toString())
    }
    fun sendOnSensorNameChange(name: String,macId: String){
        val deviceJson = JSONObject().apply {
            put("name", name)
            put("address", macId)
        }
        sendToJS("onSensorNameChange", deviceJson.toString())
    }


    fun sendSessionStarted() {
        sendToJS("onSessionStarted", "{}")
    }

    private fun parseMessage(jsonObject: JSONObject): JSBridgeMessage {
        val type = jsonObject.getString("type")
        val subType = jsonObject.optString("subType", "")
        val status = jsonObject.optString("status", "")
        
        return when (type) {
            "sensor" -> {
                when (subType) {
                    "search" -> JSBridgeMessage.Search
                    "pair" -> {
                        val address = jsonObject.optJSONObject("parameters")?.optString("sensorAddress") ?: ""
                        JSBridgeMessage.Pair(address)
                    }
                    "unPair" -> {
                        val address = jsonObject.optJSONObject("parameters")?.optString("sensorAddress") ?: ""
                        JSBridgeMessage.UnPair(address)
                    }
                    else -> JSBridgeMessage.Unknown(jsonObject.toString())
                }
            }
            "session" -> {
                when (subType) {
                    "start" -> {
                        val sessionId = jsonObject.optJSONObject("parameters")?.optString("sessionId") ?: ""
                        JSBridgeMessage.StartSession(sessionId)
                    }
                    "preRelease" -> {
                        val params = jsonObject.optJSONObject("parameters")
                        val currentBall = params?.optInt("currentBall") ?: 0
                        val totalBalls = params?.optInt("totalBalls") ?: 0
                        JSBridgeMessage.PreRelease(currentBall, totalBalls)
                    }
                    "ballRelease" -> JSBridgeMessage.BallRelease
                    "complete" -> JSBridgeMessage.SessionComplete
                    else -> JSBridgeMessage.Unknown(jsonObject.toString())
                }
            }
            else -> JSBridgeMessage.Unknown(jsonObject.toString())
        }
    }
}

// Message types for bridge communication

