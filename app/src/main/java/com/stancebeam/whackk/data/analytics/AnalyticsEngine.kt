package com.stancebeam.cc_lane.data.analytics

import com.stancebeam.cc_lane.data.analytics.models.KinematicsResult

object AnalyticsEngine {

    init {
        System.loadLibrary("analytics")
    }

    external fun initPlayer(
        batWeight: Double,
        batLength: Double,
        gripPosition: Int,
        orientationApp: Double,
        orientationAna: Double,
        userHand: Int,
        firmwareVersion: Int,
        gyroXThreshold: Double,
        dpr: Int,
    )

    external fun kinematics(
        acc: DoubleArray,
        gyro: DoubleArray,
        angle: DoubleArray,
        mag: DoubleArray,
        magMaxMin: DoubleArray,
    ): Int

    external fun getResult(): KinematicsResult
}