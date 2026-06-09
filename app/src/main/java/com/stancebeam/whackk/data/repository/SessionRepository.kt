package com.stancebeam.cc_lane.data.repository

import com.stancebeam.cc_lane.data.dao.SessionDao
import com.stancebeam.cc_lane.data.entity.Session
import com.stancebeam.cc_lane.data.model.SessionWithPlayers
import kotlinx.coroutines.flow.Flow
import javax.inject.Inject

class SessionRepository @Inject constructor(
    private val sessionDao: SessionDao,
) {

    suspend fun insert(session: Session): Long {
        return sessionDao.insert(session)
    }

    suspend fun insert(sessions: List<Session>): List<Long> {
        return sessionDao.insert(sessions)
    }

    suspend fun update(session: Session): Int {
        return sessionDao.update(session)
    }

    suspend fun delete(session: Session): Int {
        return sessionDao.delete(session)
    }

    suspend fun deleteAll(): Int {
        return sessionDao.deleteAll()
    }

    suspend fun getSession(id: String): Session? {
        return sessionDao.getSession(id)
    }

    fun getSessionWithPlayers(id: String): Flow<SessionWithPlayers> {
        return sessionDao.getSessionWithPlayers(id)
    }

    fun getSessions(): Flow<List<Session>> {
        return sessionDao.getSessions()
    }

    fun getSessionsWithPlayers(): Flow<List<SessionWithPlayers>> {
        return sessionDao.getSessionsWithPlayers()
    }
}