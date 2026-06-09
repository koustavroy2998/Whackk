package com.stancebeam.cc_lane.data.model.demomodels

import kotlinx.serialization.Serializable
import kotlinx.serialization.SerialName

@Serializable
data class SocketBle(
    @SerialName("type")
    val pType: String = "",
    @SerialName("subType")
    val pSubType: String = "",
    @SerialName("sender")
    val pSender: String = "",
    @SerialName("recipient")
    val pRecipient: String = "",
    @SerialName("laneId")
    val pLaneId: Int = 0,
    @SerialName("status")
    val pStatus: String = "",
    @SerialName("parameters")
    val pParameters: SocketParameters = SocketParameters(),
)

@Serializable
data class SubscribeSocket(
    @SerialName("sender")
    val pSender: String = "",
    @SerialName("laneId")
    val pLaneId: Int = 0,
    @SerialName("type")
    val pType: String = "",
    @SerialName("subType")
    val pSubType: String = "",
    @SerialName("status")
    val pStatus: String = "",
    @SerialName("parameters")
    val pParameters: Parameters = Parameters(),
    @SerialName("sessionToken")
    var sessionToken: String? = null,
    @SerialName("data")
    val pData: Data = Data()
)

@Serializable
data class Parameters(
    @SerialName("sessionToken")
    var sessionToken: String? = null
)

@Serializable
class Data





