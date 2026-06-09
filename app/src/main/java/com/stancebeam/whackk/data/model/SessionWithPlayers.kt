package com.stancebeam.cc_lane.data.model

import androidx.room.Embedded
import androidx.room.Junction
import androidx.room.Relation
import com.stancebeam.cc_lane.data.entity.Player
import com.stancebeam.cc_lane.data.entity.Session
import com.stancebeam.cc_lane.data.entity.SessionPlayerCrossRef

data class SessionWithPlayers(
    @Embedded val session: Session,
    @Relation(
        parentColumn = "sessionId",
        entityColumn = "playerId",
        associateBy = Junction(SessionPlayerCrossRef::class),
    ) val players: List<Player>,
)
