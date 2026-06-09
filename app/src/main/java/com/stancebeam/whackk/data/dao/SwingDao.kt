package com.stancebeam.cc_lane.data.dao

import androidx.room.Dao
import androidx.room.Insert
import androidx.room.OnConflictStrategy
import androidx.room.Query
import androidx.room.Transaction
import com.stancebeam.cc_lane.data.entity.Swing
import com.stancebeam.cc_lane.data.model.SwingWithPlayer
import kotlinx.coroutines.flow.Flow

@Dao
interface SwingDao {

    @Insert(onConflict = OnConflictStrategy.REPLACE)
    suspend fun insert(swing: Swing): Long

    @Query("DELETE FROM Swing WHERE sessionId = :sessionId;")
    suspend fun delete(sessionId: String): Int

    @Query("SELECT * FROM Swing WHERE sessionId = :sessionId ORDER BY createdAt;")
    fun getSwings(sessionId: String): Flow<List<Swing>>

    @Query("SELECT * FROM Swing ORDER BY createdAt;")
    fun getSwings(): Flow<List<Swing>>

    @Transaction
    @Query("SELECT * FROM Swing WHERE sessionId = :sessionId ORDER BY createdAt;")
    fun getSwingsAndPlayers(sessionId: String): Flow<List<SwingWithPlayer>>
}