package com.stancebeam.cc_lane.data.repository

import com.stancebeam.cc_lane.data.dao.SwingDao
import com.stancebeam.cc_lane.data.entity.Swing
import com.stancebeam.cc_lane.data.model.SwingWithPlayer
import kotlinx.coroutines.flow.Flow
import javax.inject.Inject

class SwingRepository @Inject constructor(
    private val dao: SwingDao,
) {

    suspend fun insert(swing: Swing): Long {
        return dao.insert(swing)
    }

    suspend fun delete(sessionId: String): Int {
        return dao.delete(sessionId)
    }

    fun getSwings(): Flow<List<Swing>> {
        return dao.getSwings()
    }

    fun getSwingsAndPlayers(sessionId: String): Flow<List<SwingWithPlayer>> {
        return dao.getSwingsAndPlayers(sessionId)
    }
}