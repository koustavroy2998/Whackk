package com.stancebeam.cc_lane.data.entity

import androidx.room.Entity
import androidx.room.PrimaryKey
import com.stancebeam.cc_lane.data.util.SessionDefaults
import com.stancebeam.cc_lane.util.randomId
import com.stancebeam.cc_lane.util.timestamp

@Entity
data class Session(
    @PrimaryKey val sessionId: String = randomId(),
    val userId: String,
    val sessionStatus: String = SessionDefaults.Status.STARTED,
    val sessionTag: String? = null,
    val startedAt: Long = timestamp(),
    val endedAt: Long? = null,
    val duration: Int = 0,
    val totalSwings: Int = 0,
    val createdAt: Long = timestamp(),
    val updatedAt: Long = createdAt,
)