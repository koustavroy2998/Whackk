package com.stancebeam.cc_lane.data.util

sealed class SessionDefaults {

    data object Status : SessionDefaults() {

        const val STARTED = "started"
        const val COMPLETED = "completed"
    }
}