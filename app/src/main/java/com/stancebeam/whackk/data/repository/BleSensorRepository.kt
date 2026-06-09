package com.stancebeam.cc_lane.data.repository

import com.stancebeam.cc_lane.data.entity.BleSensor

// In repository folder
interface BleSensorRepository {
    suspend fun getSensorByMacAddress(macAddress: String): BleSensor?
    suspend fun getAllActiveSensors(): List<BleSensor>
    suspend fun getAllSensors(): List<BleSensor>
    suspend fun saveSensor(sensor: BleSensor)
    suspend fun updateSensorName(macAddress: String, name: String)
    suspend fun updateLastDetectedTime(macAddress: String)
    suspend fun deleteSensor(macAddress: String)
    suspend fun getSensorNameOrDefault(macAddress: String, defaultName: String): String
}
