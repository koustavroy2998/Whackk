package com.stancebeam.cc_lane.data.repository

import com.stancebeam.cc_lane.data.dao.BleSensorDao
import com.stancebeam.cc_lane.data.entity.BleSensor
import javax.inject.Inject
import javax.inject.Singleton

// In repository folder
@Singleton
class BleSensorRepositoryImpl @Inject constructor(
    private val bleSensorDao: BleSensorDao
) : BleSensorRepository {
    
    override suspend fun getSensorByMacAddress(macAddress: String): BleSensor? {
        return bleSensorDao.getSensorByMacAddress(macAddress)
    }
    
    override suspend fun getAllActiveSensors(): List<BleSensor> {
        return bleSensorDao.getAllActiveSensors()
    }
    
    override suspend fun getAllSensors(): List<BleSensor> {
        return bleSensorDao.getAllSensors()
    }
    
    override suspend fun saveSensor(sensor: BleSensor) {
        bleSensorDao.insertSensor(sensor)
    }
    
    override suspend fun updateSensorName(macAddress: String, name: String) {
        bleSensorDao.updateSensorName(macAddress, name)
    }
    
    override suspend fun updateLastDetectedTime(macAddress: String) {
        bleSensorDao.updateLastDetectedTime(macAddress, System.currentTimeMillis())
    }
    
    override suspend fun deleteSensor(macAddress: String) {
        bleSensorDao.deleteSensor(macAddress)
    }
    
    override suspend fun getSensorNameOrDefault(macAddress: String, defaultName: String): String {
        val sensor = bleSensorDao.getSensorByMacAddress(macAddress)
        return sensor?.customName ?: defaultName
    }
}
