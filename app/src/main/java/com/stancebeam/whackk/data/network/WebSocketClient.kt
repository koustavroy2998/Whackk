package com.stancebeam.cc_lane.data.network

import android.util.Log
import com.stancebeam.cc_lane.data.model.demomodels.Data
import com.stancebeam.cc_lane.data.model.demomodels.Parameters
import com.stancebeam.cc_lane.data.model.demomodels.SocketBle
import com.stancebeam.cc_lane.data.model.demomodels.SubscribeSocket
import com.stancebeam.cc_lane.data.network.model.OutgoingMessage
import com.stancebeam.cc_lane.data.repository.PreferencesRepository
import com.stancebeam.cc_lane.data.util.Constants.LANE_APP_LANE_1
import com.stancebeam.cc_lane.data.util.Constants.STATUS_INIT
import com.stancebeam.cc_lane.util.WebSocketState
import io.ktor.client.HttpClient
import io.ktor.client.plugins.websocket.webSocketSession
import io.ktor.websocket.Frame
import io.ktor.websocket.WebSocketSession
import io.ktor.websocket.close
import io.ktor.websocket.readText
import kotlinx.coroutines.DelicateCoroutinesApi
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.asSharedFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.consumeAsFlow
import kotlinx.coroutines.flow.emitAll
import kotlinx.coroutines.flow.filterIsInstance
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.flow.mapNotNull
import kotlinx.coroutines.launch
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import kotlin.time.Duration.Companion.seconds

private const val TAG = "WebSocketClient"

private const val HOST = "192.168.0.111"
private const val PORT = 9090
private const val PATH = "ws"

class WebSocketClient(
    private val client: HttpClient,
    private val preferences: PreferencesRepository,
) {

    private val _state = MutableStateFlow<WebSocketState>(WebSocketState.Connecting)
    val state = _state.asStateFlow()
    private var session: WebSocketSession? = null
    private val _messages = MutableSharedFlow<SocketBle>()
    val messages = _messages.asSharedFlow()
    val jsonParser = Json {
        ignoreUnknownKeys = true
    }

    // Store the session token received in WELCOME
    private var sessionToken: String? = null

    suspend fun connect() {
        _state.value = WebSocketState.Connecting
        try {
            _state.value = WebSocketState.Connected
            session = client.webSocketSession(
                host = HOST, port = PORT, path = PATH
            )
            Log.d(TAG, "connect: Connected to WebSocket")

            // Subscribe immediately
            sendSubscribe(
                SubscribeSocket(
                    pSender = LANE_APP_LANE_1,
                    pLaneId = 1,
                    pType = "CHANNEL",
                    pSubType = "SUBSCRIBE",
                    pParameters = Parameters(),
                    pStatus = STATUS_INIT,
                    pData = Data()
                )
            )

            session?.incoming?.consumeAsFlow()
                ?.filterIsInstance<Frame.Text>()
                ?.mapNotNull {
                    val json = it.readText()
                    Log.d(TAG, "connect: Received = $json")
                    val socketMessage = jsonParser.decodeFromString<SocketBle>(json)

                    // Save sessionToken if it's a WELCOME message
                    if (socketMessage.pType == "USER" && socketMessage.pSubType == "WELCOME") {
                        val token = socketMessage.pParameters?.sessionToken
                        if (!token.isNullOrEmpty()) {
                            sessionToken = token
                            Log.d(TAG, "Session token saved = $sessionToken")
                        }
                    }
                    socketMessage
                }
                ?.let {
                    _messages.emitAll(it)
                }

            _state.value = WebSocketState.Disconnected
            Log.d(TAG, "connect: Disconnected from WebSocket")
        } catch (e: Exception) {
            Log.e(TAG, "connect: ", e)
        }
        Log.d(TAG, "connect: Retrying to connect in 3 seconds")
        delay(3.seconds)
        connect()
    }

    suspend fun sendMessage(message: OutgoingMessage): Boolean {
        try {
            session?.outgoing?.let {
                val currentUser = preferences.currentUserFlow.first()
                message.accessToken = currentUser?.accessToken
                message.sessionToken = sessionToken // Attach session token

                val json = Json.encodeToString(message)
                it.send(Frame.Text(json))
                Log.d(TAG, "sendMessage: Sent = $json")
                return true
            }
            throw Exception("Session is null")
        } catch (e: Exception) {
            Log.e(TAG, "sendMessage: ", e)
            return false
        }
    }

    suspend fun sendMessage2(message: SocketBle): Boolean {
        try {
            session?.outgoing?.let {
                // Inject sessionToken inside parameters if needed
                val enrichedMessage = message.copy(
                    pParameters = message.pParameters.copy(
                        sessionToken = sessionToken ?: message.pParameters.sessionToken
                    )
                )
                val json = Json.encodeToString(enrichedMessage)
                it.send(Frame.Text(json))
                Log.d(TAG, "sendMessage2: Sent = $json")
                return true
            }
            throw Exception("Session is null")
        } catch (e: Exception) {
            Log.e(TAG, "sendMessage2: ", e)
            return false
        }
    }

    suspend fun sendSubscribe(message: SubscribeSocket): Boolean {
        try {
            session?.outgoing?.let {
                val enrichedMessage = message.copy(
                    pParameters = message.pParameters.copy(
                        sessionToken = sessionToken ?: message.pParameters.sessionToken
                    )
                )
                val json = Json.encodeToString(enrichedMessage)
                it.send(Frame.Text(json))
                Log.d(TAG, "sendSubscribe: Sent = $json")
                return true
            }
            throw Exception("Session is null")
        } catch (e: Exception) {
            Log.e(TAG, "sendSubscribe: ", e)
            return false
        }
    }

    @OptIn(DelicateCoroutinesApi::class)
    fun disconnect() {
        GlobalScope.launch(Dispatchers.IO) {
            try {
                session?.close()
                session = null
                Log.d(TAG, "disconnect: Disconnected from WebSocket")
            } catch (e: Exception) {
                Log.e(TAG, "disconnect: ", e)
            }
        }
    }
}
