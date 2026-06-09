package com.stancebeam.cc_lane.data.network.repository

import com.parse.ParseUser
import com.parse.coroutines.parseLogIn
import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException
import kotlin.coroutines.suspendCoroutine

class AuthRepository {

    fun getCurrentUser(): ParseUser? {
        return ParseUser.getCurrentUser()
    }

    suspend fun signIn(email: String, password: String): Result<ParseUser> {
        return try {
            val user = parseLogIn(email, password)
            Result.success(user)
        } catch (e: Exception) {
            Result.failure(e)
        }
    }

    suspend fun signOut() {
        return suspendCoroutine { continuation ->
            ParseUser.logOutInBackground {
                if (it == null) continuation.resume(Unit)
                else continuation.resumeWithException(it)
            }
        }
    }
}