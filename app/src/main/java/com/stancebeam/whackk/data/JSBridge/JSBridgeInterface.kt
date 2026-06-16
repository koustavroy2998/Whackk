package com.stancebeam.cc_lane.data.JSBridge

import android.webkit.JavascriptInterface
import android.util.Log
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class JSBridgeInterface @Inject constructor(
    private val bridgeController: JSBridgeController
) {
    private val TAG = "JSBridgeInterface"

    @JavascriptInterface
    fun sendMessage(messageJson: String) {
        Log.d(TAG, "Received message from JS: $messageJson")
        bridgeController.handleMessage(messageJson)
    }


    @JavascriptInterface
    fun changeSensorName(name: String,macId:String) {
        Log.d(TAG, "Name change : $name -- $macId")
        bridgeController.handleSensorNameChange(name = name, macId = macId)
    }



    @JavascriptInterface
    fun search() {
        Log.d(TAG, "Search triggered from JS")
        bridgeController.handleSearch()
    }

    @JavascriptInterface
    fun pair(sensorAddress: String) {
        Log.d(TAG, "Pair triggered from JS: $sensorAddress")
        bridgeController.handlePair(sensorAddress)
    }

    @JavascriptInterface
    fun unPair(sensorAddress: String) {
        Log.d(TAG, "UnPair triggered from JS: $sensorAddress")
        bridgeController.handleUnPair(sensorAddress)
    }

    @JavascriptInterface
    fun startSession(sessionId: String) {
        Log.d(TAG, "Start session triggered from JS: $sessionId")
        bridgeController.handleStartSession(sessionId)
    }

    @JavascriptInterface
    fun preRelease(currentBall: Int, totalBalls: Int) {
        Log.d(TAG, "Pre-release triggered from JS: $currentBall/$totalBalls")
        bridgeController.handlePreRelease(currentBall, totalBalls)
    }

    @JavascriptInterface
    fun ballRelease() {
        Log.d(TAG, "Ball release triggered from JS")
        bridgeController.handleBallRelease()
    }

    @JavascriptInterface
    fun complete() {
        Log.d(TAG, "Session complete triggered from JS")
        bridgeController.handleSessionComplete()
    }

    @JavascriptInterface
    fun logout() {
        Log.d(TAG, "Logout triggered from JS")
        bridgeController.handleLogout()
    }

    @JavascriptInterface
    fun startSpeechRecognition(language: String = "en-US") {
        Log.d(TAG, "STT start triggered from JS: $language")
        bridgeController.handleStartSTT(language)
    }

    @JavascriptInterface
    fun stopSpeechRecognition() {
        Log.d(TAG, "STT stop triggered from JS")
        bridgeController.handleStopSTT()
    }
}
