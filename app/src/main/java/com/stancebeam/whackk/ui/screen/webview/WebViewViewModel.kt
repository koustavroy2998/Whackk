package com.stancebeam.cc_lane.ui.screen.webview

import android.util.Log
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.stancebeam.cc_lane.data.network.WebSocketClient
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.collectLatest
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import javax.inject.Inject

@HiltViewModel
class WebViewViewModel @Inject constructor(
    private val webSocketClient: WebSocketClient,
): ViewModel() {

    private val _uiState = MutableStateFlow(WebViewState())
    val uiState = _uiState.asStateFlow()


    init {

        viewModelScope.launch {
            webSocketClient.messages.collectLatest { message ->
                Log.d("WebViewViewModel", "guest: $message")
                if(message.pType=="SENSOR_CONFIG"){
                    _uiState.update { it.copy(showConnectionSheet = true) }
                }
            }
        }
    }


    fun sendMessage(){
        viewModelScope.launch {
//            webSocketClient.sendMessage2(
//                SocketBle(
//                    pType = "new message",
//                    pSubType = "",
//                    pStatus = ""
//                )
//            )
        }
    }


}