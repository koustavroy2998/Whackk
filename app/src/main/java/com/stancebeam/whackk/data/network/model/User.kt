package com.stancebeam.cc_lane.data.network.model

import kotlinx.serialization.Serializable

@Serializable
data class User(
    val userId: String = "123",
    val fullName: String = "Anikesh",
    val email: String = "abc@gmail.com",
    val userType: String = "player",
    val accessToken: String = "accessToken",
    val expiresAt: Long = 0L,
    val createdAt: Long = 0L,
    val updatedAt: Long = 0L,
)