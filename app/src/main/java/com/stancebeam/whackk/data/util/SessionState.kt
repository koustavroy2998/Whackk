package com.stancebeam.cc_lane.data.util

sealed class SessionState {

    data object WritingDataFormat : SessionState()
    data object EnablingDataNotification : SessionState()
    data object Started : SessionState()
    data class Error(val message: String?) : SessionState()
}