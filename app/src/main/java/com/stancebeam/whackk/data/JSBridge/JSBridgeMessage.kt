package com.stancebeam.cc_lane.data.JSBridge

sealed class JSBridgeMessage {
    object Search : JSBridgeMessage()

    data class Pair(val sensorAddress: String) : JSBridgeMessage()
    data class SensorName(val name:String,val macId:String) : JSBridgeMessage()
    data class UnPair(val sensorAddress: String) : JSBridgeMessage()
    data class StartSession(val sessionId: String) : JSBridgeMessage()
    data class PreRelease(val currentBall: Int, val totalBalls: Int) : JSBridgeMessage()
    object BallRelease : JSBridgeMessage()
    object SessionComplete : JSBridgeMessage()

    object Logout : JSBridgeMessage()
    data class Unknown(val rawMessage: String) : JSBridgeMessage()
}