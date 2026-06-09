package com.stancebeam.cc_lane.data.dao

import androidx.room.Dao
import androidx.room.Delete
import androidx.room.Insert
import androidx.room.OnConflictStrategy
import androidx.room.Query
import androidx.room.Transaction
import androidx.room.Update
import com.stancebeam.cc_lane.data.entity.Session
import com.stancebeam.cc_lane.data.model.SessionWithPlayers
import kotlinx.coroutines.flow.Flow

@Dao
interface SessionDao {

    @Insert(onConflict = OnConflictStrategy.REPLACE)
    suspend fun insert(session: Session): Long

    @Insert(onConflict = OnConflictStrategy.REPLACE)
    suspend fun insert(sessions: List<Session>): List<Long>

    @Update
    suspend fun update(session: Session): Int

    @Delete
    suspend fun delete(session: Session): Int

    @Query("DELETE FROM Session;")
    suspend fun deleteAll(): Int

    @Query("SELECT * FROM Session WHERE sessionId = :id;")
    suspend fun getSession(id: String): Session?

    @Transaction
    @Query("SELECT * FROM Session WHERE sessionId = :id;")
    fun getSessionWithPlayers(id: String): Flow<SessionWithPlayers>

    @Query("SELECT * FROM Session ORDER BY startedAt DESC;")
    fun getSessions(): Flow<List<Session>>

    @Transaction
    @Query("SELECT * FROM Session ORDER BY startedAt DESC;")
    fun getSessionsWithPlayers(): Flow<List<SessionWithPlayers>>
}