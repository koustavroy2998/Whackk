package com.stancebeam.cc_lane.data.stt

import android.Manifest
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.speech.RecognitionListener
import android.speech.RecognizerIntent
import android.speech.SpeechRecognizer
import android.util.Log
import com.stancebeam.cc_lane.data.JSBridge.JSBridgeController
import dagger.hilt.android.qualifiers.ApplicationContext
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class STTManager @Inject constructor(
    @ApplicationContext private val context: Context,
    private val jsBridgeController: JSBridgeController
) {
    private val TAG = "STTManager"
    private val mainHandler = Handler(Looper.getMainLooper())
    private var speechRecognizer: SpeechRecognizer? = null

    fun startListening(language: String = "en-US") {
        if (context.checkSelfPermission(Manifest.permission.RECORD_AUDIO) != PackageManager.PERMISSION_GRANTED) {
            jsBridgeController.sendSTTError("PERMISSION_DENIED")
            return
        }
        if (!SpeechRecognizer.isRecognitionAvailable(context)) {
            jsBridgeController.sendSTTError("NOT_AVAILABLE")
            return
        }
        mainHandler.post {
            destroyRecognizer()
            speechRecognizer = SpeechRecognizer.createSpeechRecognizer(context).apply {
                setRecognitionListener(createListener())
            }
            val intent = Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH).apply {
                putExtra(RecognizerIntent.EXTRA_LANGUAGE_MODEL, RecognizerIntent.LANGUAGE_MODEL_FREE_FORM)
                putExtra(RecognizerIntent.EXTRA_LANGUAGE, language)
                putExtra(RecognizerIntent.EXTRA_PARTIAL_RESULTS, true)
                putExtra(RecognizerIntent.EXTRA_MAX_RESULTS, 1)
                // Stop after 1.5s of silence
                putExtra(RecognizerIntent.EXTRA_SPEECH_INPUT_COMPLETE_SILENCE_LENGTH_MILLIS, 1500L)
                putExtra(RecognizerIntent.EXTRA_SPEECH_INPUT_POSSIBLY_COMPLETE_SILENCE_LENGTH_MILLIS, 1500L)
            }
            speechRecognizer?.startListening(intent)
            Log.d(TAG, "STT started, language=$language")
        }
    }

    fun stopListening() {
        mainHandler.post {
            speechRecognizer?.stopListening()
            destroyRecognizer()
            Log.d(TAG, "STT stopped")
        }
    }

    private fun destroyRecognizer() {
        speechRecognizer?.destroy()
        speechRecognizer = null
    }

    private fun createListener() = object : RecognitionListener {
        override fun onReadyForSpeech(params: Bundle?) {}
        override fun onBeginningOfSpeech() {}
        override fun onRmsChanged(rmsdB: Float) {}
        override fun onBufferReceived(buffer: ByteArray?) {}
        override fun onEndOfSpeech() {
            // Finalize immediately on pause instead of waiting for the recognizer's own timeout
            speechRecognizer?.stopListening()
        }
        override fun onEvent(eventType: Int, params: Bundle?) {}

        override fun onResults(results: Bundle?) {
            val transcript = results
                ?.getStringArrayList(SpeechRecognizer.RESULTS_RECOGNITION)
                ?.firstOrNull()
                .orEmpty()
            Log.d(TAG, "STT result: $transcript")
            jsBridgeController.sendSTTResult(transcript, isFinal = true)
            destroyRecognizer()
        }

        override fun onPartialResults(partialResults: Bundle?) {
            val transcript = partialResults
                ?.getStringArrayList(SpeechRecognizer.RESULTS_RECOGNITION)
                ?.firstOrNull()
                .orEmpty()
            if (transcript.isNotEmpty()) {
                Log.d(TAG, "STT partial: $transcript")
                jsBridgeController.sendSTTResult(transcript, isFinal = false)
            }
        }

        override fun onError(error: Int) {
            val code = mapError(error)
            Log.e(TAG, "STT error: $code ($error)")
            jsBridgeController.sendSTTError(code)
            destroyRecognizer()
        }
    }

    private fun mapError(error: Int) = when (error) {
        SpeechRecognizer.ERROR_AUDIO -> "AUDIO_ERROR"
        SpeechRecognizer.ERROR_CLIENT -> "CLIENT_ERROR"
        SpeechRecognizer.ERROR_INSUFFICIENT_PERMISSIONS -> "PERMISSION_DENIED"
        SpeechRecognizer.ERROR_NETWORK -> "NETWORK_ERROR"
        SpeechRecognizer.ERROR_NETWORK_TIMEOUT -> "NETWORK_TIMEOUT"
        SpeechRecognizer.ERROR_NO_MATCH -> "NO_MATCH"
        SpeechRecognizer.ERROR_RECOGNIZER_BUSY -> "RECOGNIZER_BUSY"
        SpeechRecognizer.ERROR_SERVER -> "SERVER_ERROR"
        SpeechRecognizer.ERROR_SPEECH_TIMEOUT -> "SPEECH_TIMEOUT"
        else -> "UNKNOWN_ERROR"
    }
}
