package com.stancebeam.cc_lane.data.network.model

import android.util.Log
import kotlinx.serialization.Serializable
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.JsonElement
import kotlinx.serialization.json.decodeFromJsonElement

@Serializable
data class IncomingMessage(
    val command: String,
    val fromServiceId: String? = null,
    val fromClientId: String? = null,
    val payload: JsonElement? = null,
) {

    inline fun <reified T> getPayload(): T? {
        val withUnknownKeys = Json {
            ignoreUnknownKeys = true
        }
        return try {
            payload?.let {
                withUnknownKeys.decodeFromJsonElement(payload)
            }
        } catch (e: Exception) {
            Log.e("IncomingMessage", "getPayload: ", e)
            null
        }
    }
}