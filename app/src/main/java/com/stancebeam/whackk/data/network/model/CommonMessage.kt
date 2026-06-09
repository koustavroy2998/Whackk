package com.stancebeam.cc_lane.data.network.model
import kotlinx.serialization.Serializable

import kotlinx.serialization.SerialName

@Serializable
data class CommonMessage(
    @SerialName("status")
    val pStatus: String = "",
    @SerialName("message")
    val pMessage: String = "",
    @SerialName("data")
    val pData: Data = Data(),
    @SerialName("statusCode")
    val pStatusCode: String = ""
)


@Serializable
data class Data(
    @SerialName("name")
    val pName: String = "",
    @SerialName("strikerId")
    val pStrikerId: String = "",
    @SerialName("status")
    val pStatus: String = "",

)


