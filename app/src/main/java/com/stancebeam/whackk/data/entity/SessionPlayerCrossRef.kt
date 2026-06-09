package com.stancebeam.cc_lane.data.entity

import androidx.room.Entity

@Entity(primaryKeys = ["sessionId", "playerId"])
data class SessionPlayerCrossRef(
    val sessionId: String,
    val playerId: String,
)