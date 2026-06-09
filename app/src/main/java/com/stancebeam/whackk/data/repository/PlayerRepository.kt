package com.stancebeam.cc_lane.data.repository

import com.stancebeam.cc_lane.data.dao.PlayerDao
import com.stancebeam.cc_lane.data.entity.Player
import kotlinx.coroutines.flow.Flow
import javax.inject.Inject

class PlayerRepository @Inject constructor(
    private val dao: PlayerDao,
) {

    suspend fun insert(player: Player): Long {
        return dao.insert(player)
    }

    suspend fun insert(players: List<Player>): List<Long> {
        return dao.insert(players)
    }

    suspend fun update(player: Player): Int {
        return dao.update(player)
    }

    suspend fun delete(player: Player): Int {
        return dao.delete(player)
    }

    suspend fun deleteAll(): Int {
        return dao.deleteAll()
    }

    fun getPlayers(): Flow<List<Player>> {
        return dao.getPlayers()
    }
}