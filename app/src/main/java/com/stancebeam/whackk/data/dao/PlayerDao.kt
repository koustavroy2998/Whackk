package com.stancebeam.cc_lane.data.dao

import androidx.room.Dao
import androidx.room.Delete
import androidx.room.Insert
import androidx.room.OnConflictStrategy
import androidx.room.Query
import androidx.room.Update
import com.stancebeam.cc_lane.data.entity.Player
import kotlinx.coroutines.flow.Flow

@Dao
interface PlayerDao {

    @Insert(onConflict = OnConflictStrategy.REPLACE)
    suspend fun insert(player: Player): Long

    @Insert(onConflict = OnConflictStrategy.REPLACE)
    suspend fun insert(players: List<Player>): List<Long>

    @Update
    suspend fun update(player: Player): Int

    @Delete
    suspend fun delete(player: Player): Int

    @Query("DELETE FROM Player;")
    suspend fun deleteAll(): Int

    @Query("SELECT * FROM Player ORDER BY fullName;")
    fun getPlayers(): Flow<List<Player>>
}