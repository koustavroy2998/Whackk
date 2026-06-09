package com.stancebeam.cc_lane.ui

import android.annotation.SuppressLint
import android.content.Context
import android.os.Build
import android.provider.Settings
import android.util.Log
import androidx.annotation.RequiresPermission
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.stancebeam.cc_lane.data.bluetooth.BluetoothController
import com.stancebeam.cc_lane.data.network.WebSocketClient
import com.stancebeam.cc_lane.data.repository.PreferencesRepository
import com.stancebeam.cc_lane.data.util.PermissionController
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import org.json.JSONObject
import java.io.OutputStreamWriter
import java.net.HttpURLConnection
import java.net.URL
import java.util.UUID
import javax.inject.Inject

@HiltViewModel
class MainViewModel @Inject constructor(
    private val permissionController: PermissionController,
    private val bluetoothController: BluetoothController,
    private val webSocketClient: WebSocketClient,
    preferencesRepository: PreferencesRepository,
) : ViewModel() {

    val hasPermissions = permissionController.hasPermissions
    val requiredPermissions = permissionController.requiredPermissions
    val isBluetoothEnabled get() = bluetoothController.isBluetoothEnabled()
    val darkTheme = preferencesRepository.darkThemeFlow

    init {
        viewModelScope.launch {
            webSocketClient.connect()
        }
    }

    fun onPermissionsResult(result: Map<String, Boolean>) {
        permissionController.onPermissionsResult(result)
    }


    @SuppressLint("HardwareIds")
    fun sendLaneDataToGoogleSheet(
        context: Context,
        laneNumber: String,
        appVersion: String,
        testerEmail: String
    ) {
        val deviceId = Settings.Secure.getString(context.contentResolver, Settings.Secure.ANDROID_ID)
        Log.d("ANDROID ID->","$deviceId")
        val urlString = "https://script.google.com/macros/s/AKfycbwmIOhcZfP7Ee9Rzy_3lv6V6wdej0n92Keexjbt0HZA2UqX4pHCeGPRX59oPpRY-_YP8A/exec" // replace with your URL

        CoroutineScope(Dispatchers.IO).launch {
            try {
                val json = JSONObject()
                json.put("lane", laneNumber)
                json.put("deviceId", deviceId)
                json.put("appVersion", appVersion)
                json.put("testerEmail", testerEmail)

                val url = URL(urlString)
                val conn = url.openConnection() as HttpURLConnection
                conn.requestMethod = "POST"
                conn.setRequestProperty("Content-Type", "application/json")
                conn.doOutput = true

                val writer = OutputStreamWriter(conn.outputStream)
                writer.write(json.toString())
                writer.flush()
                writer.close()

                val responseCode = conn.responseCode
                println("✅ Lane data sent: HTTP $responseCode")

                conn.disconnect()
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }
    }

    override fun onCleared() {
        super.onCleared()
        webSocketClient.disconnect()
    }
}