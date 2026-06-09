package com.stancebeam.cc_lane.data.dao

import androidx.room.Dao
import androidx.room.Delete
import androidx.room.Insert
import androidx.room.OnConflictStrategy
import androidx.room.Query
import androidx.room.Transaction
import androidx.room.Update
import com.stancebeam.cc_lane.data.entity.Striker
import com.stancebeam.cc_lane.data.model.StrikerWithPlayer
import kotlinx.coroutines.flow.Flow

@Dao
interface StrikerDao {

    @Insert(onConflict = OnConflictStrategy.REPLACE)
    suspend fun insert(striker: Striker): Long

    @Update
    suspend fun update(striker: Striker): Int

    @Delete
    suspend fun delete(striker: Striker): Int

    @Query("SELECT * FROM Striker ORDER BY name;")
    fun getFlow(): Flow<List<Striker>>

    @Transaction
    @Query("SELECT * FROM Striker ORDER BY name;")
    fun getStrikers(): Flow<List<StrikerWithPlayer>>
}