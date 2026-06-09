package com.stancebeam.cc_lane.data.model.demomodels

import kotlinx.serialization.SerialName
import kotlinx.serialization.Serializable

@Serializable
data class SocketParameters(
    @SerialName("sessionId")
    val pSessionId: String = "",
    @SerialName("currentBall")
    val pCurrentBall: Int = 0,
    @SerialName("totalBalls")
    val pTotalBalls: Int = 0,
    @SerialName("sensorName")
    val pSensorName: String = "",
    @SerialName("sensorAddress")
    val pSensorAddress: String = "",
    @SerialName("pairedName")
    val pPairedName: String = "",
    @SerialName("pairedAddress")
    val pPairedAddress: String = "",
    @SerialName("sensorData")
    val pSensorData: SensorData = SensorData(),
    @SerialName("device_list")
    val deviceList: List<DeviceListItem> = listOf(DeviceListItem()),
    @SerialName("sessionToken")
    val sessionToken: String = ""
)

@Serializable
data class DeviceListItem(
    @SerialName("sensorName")
    val pSensorName: String = "",
    @SerialName("sensorAddress")
    val pSensorAddress: String = ""
)

@Serializable
data class SensorData(
    @SerialName("power")
    val power: Double = 0.0,
    @SerialName("powerAtImpact")
    val powerAtImpact: Double = 0.0,
    @SerialName("speed")
    val pSpeed: Double = 0.0,
    @SerialName("speedAtImpact")
    val pSpeedAtImpact: Double = 0.0,
    @SerialName("efficiency")
    val pEfficiency: Double = 0.0,
    @SerialName("backLift")
    val pBackLift: Double = 0.0,
    @SerialName("downSwing")
    val pDownSwing: Double = 0.0,
    @SerialName("followThrough")
    val pFollowThrough: Double = 0.0,
    @SerialName("timeToImpact")
    val timeToImpact: Double = 0.0,
    @SerialName("batFace")
    val batFace: Double = 0.0,
    @SerialName("shotType")
    val pShotType: Int = 0,
)
