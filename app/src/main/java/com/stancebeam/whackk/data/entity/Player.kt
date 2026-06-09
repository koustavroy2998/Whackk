package com.stancebeam.cc_lane.data.entity

import androidx.room.Entity
import androidx.room.PrimaryKey
import com.stancebeam.cc_lane.data.util.PlayerDefaults
import com.stancebeam.cc_lane.util.timestamp
import kotlinx.serialization.Serializable

@Serializable
@Entity
data class Player(
    @PrimaryKey val playerId: String,
    val fullName: String,
    val email: String,
    val gender: String? = null,
    val dob: Long? = null,
    val profilePicture: String? = null,
    val phone: String? = null,
    val height: Int = PlayerDefaults.HEIGHT,
    val weight: Int = PlayerDefaults.WEIGHT,
    val battingHand: Int = PlayerDefaults.BattingHand.RIGHT,
    val gripPosition: Int = PlayerDefaults.GripPosition.MEDIUM,
    val bowlingArm: String = PlayerDefaults.BowlingArm.RIGHT,
    val bowlingType: String = PlayerDefaults.BowlingType.MEDIUM,
    val createdAt: Long = timestamp(),
    val updatedAt: Long = createdAt,
)