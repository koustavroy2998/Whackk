package com.stancebeam.cc_lane.data.entity

import androidx.room.Entity
import androidx.room.PrimaryKey

@Entity
data class Striker(
    @PrimaryKey val address: String,
    val name: String?,
    val playerId: String? = null,
)