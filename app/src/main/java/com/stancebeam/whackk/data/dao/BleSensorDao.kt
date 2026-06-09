package com.stancebeam.cc_lane.data.dao

import androidx.room.Dao
import androidx.room.Insert
import androidx.room.OnConflictStrategy
import androidx.room.Query
import androidx.room.Update
import com.stancebeam.cc_lane.data.entity.BleSensor

// In dao folder
@Dao
interface BleSensorDao {
    
    @Query("SELECT * FROM ble_sensors WHERE macAddress = :macAddress LIMIT 1")
    suspend fun getSensorByMacAddress(macAddress: String): BleSensor?
    
    @Query("SELECT * FROM ble_sensors WHERE isActive = 1")
    suspend fun getAllActiveSensors(): List<BleSensor>
    
    @Query("SELECT * FROM ble_sensors")
    suspend fun getAllSensors(): List<BleSensor>
    
    @Insert(onConflict = OnConflictStrategy.REPLACE)
    suspend fun insertSensor(sensor: BleSensor)
    
    @Update
    suspend fun updateSensor(sensor: BleSensor)
    
    @Query("UPDATE ble_sensors SET customName = :name WHERE macAddress = :macAddress")
    suspend fun updateSensorName(macAddress: String, name: String)
    
    @Query("UPDATE ble_sensors SET lastDetectedTime = :timestamp WHERE macAddress = :macAddress")
    suspend fun updateLastDetectedTime(macAddress: String, timestamp: Long)
    
    @Query("DELETE FROM ble_sensors WHERE macAddress = :macAddress")
    suspend fun deleteSensor(macAddress: String)
    
    @Query("UPDATE ble_sensors SET isActive = :isActive WHERE macAddress = :macAddress")
    suspend fun updateSensorStatus(macAddress: String, isActive: Boolean)
}
