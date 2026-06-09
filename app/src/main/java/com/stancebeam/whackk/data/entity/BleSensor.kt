package com.stancebeam.cc_lane.data.entity

import androidx.room.Entity
import androidx.room.PrimaryKey

// In entity folder
@Entity(tableName = "ble_sensors")
data class BleSensor(
    @PrimaryKey
    val macAddress: String,
    val customName: String,
    val lastDetectedTime: Long = System.currentTimeMillis(),
    val isActive: Boolean = true
)
