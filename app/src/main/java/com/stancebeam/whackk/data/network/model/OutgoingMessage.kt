package com.stancebeam.cc_lane.data.network.model

import kotlinx.serialization.Serializable

@Serializable
data class OutgoingMessage(
    val command: String,
    val toServiceId: String,
    val payload: Map<String, String>? = null,
    var accessToken: String? = null,
    var sessionToken:String?=null,
)