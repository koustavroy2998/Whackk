package com.stancebeam.cc_lane.data.entity

import androidx.room.Entity
import androidx.room.PrimaryKey
import com.stancebeam.cc_lane.util.timestamp

@Entity
data class Swing(
    @PrimaryKey val swingId: String = "",
    val playerId: String,
    val sessionId: String,
    val swingNumber: Int,
    val power: Double,
    val powerAtImpact: Double,
    val speed: Double,
    val speedAtImpact: Double,
    val efficiency: Double,
    val backLift: Double,
    val downswing: Double,
    val followThrough: Double,
    val timeToImpact: Double,
    val batFace: Double,
    val shotType: Int,
    val createdAt: Long = timestamp(),
    val updatedAt: Long = createdAt,
)