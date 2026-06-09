package com.stancebeam.cc_lane.util

sealed class WebSocketState {

    data object Connecting : WebSocketState()
    data object Connected : WebSocketState()
    data object Disconnected : WebSocketState()
}