package com.stancebeam.cc_lane.data.network.api

import com.parse.ParseObject
import com.parse.ParseQuery
import com.parse.coroutines.suspendFind

class PlayerApi {

    companion object {

        private const val IDENTITY_INFO = "IdentityInfo"
    }

    suspend fun getPlayers(userId: String, branch: String? = null): List<ParseObject> {
        return ParseQuery.getQuery<ParseObject>(IDENTITY_INFO)
            .apply {
                whereEqualTo("identity", "user.profile")
                whereEqualTo("userId", userId)
                branch?.let {
                    whereEqualTo("branch", it)
                }
            }
            .suspendFind()
    }
}