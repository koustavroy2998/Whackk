package com.stancebeam.cc_lane.data.repository

import com.stancebeam.cc_lane.data.dao.StrikerDao
import com.stancebeam.cc_lane.data.entity.Striker
import com.stancebeam.cc_lane.data.model.StrikerWithPlayer
import kotlinx.coroutines.flow.Flow
import javax.inject.Inject

class StrikerRepository @Inject constructor(
    private val dao: StrikerDao,
) {

    suspend fun insert(striker: Striker): Long {
        return dao.insert(striker)
    }

    suspend fun update(striker: Striker): Int {
        return dao.update(striker)
    }

    suspend fun delete(striker: Striker): Int {
        return dao.delete(striker)
    }

    fun getStrikers(): Flow<List<StrikerWithPlayer>> {
        return dao.getStrikers()
    }
}