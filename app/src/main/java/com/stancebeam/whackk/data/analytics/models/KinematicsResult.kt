package com.stancebeam.cc_lane.data.analytics.models

data class KinematicsResult(
    val power: Double,
    val powerAtImpact: Double,
    val speed: Double,
    val speedAtImpact: Double,
    val efficiency: Double,
    val backLift: Double,
    val downswing: Double,
    val followThrough: Double,
    val timeToImpact: Double,
    val batFace: Double,
    val shotType: Int,
)