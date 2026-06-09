package com.stancebeam.cc_lane.data.model

import androidx.room.Embedded
import androidx.room.Relation
import com.stancebeam.cc_lane.data.entity.Player
import com.stancebeam.cc_lane.data.entity.Striker

data class StrikerWithPlayer(
    @Embedded val striker: Striker,
    @Relation(
        parentColumn = "playerId",
        entityColumn = "playerId",
    ) val player: Player?,
)