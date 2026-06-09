package com.stancebeam.cc_lane.data.util

object Constants {

    const val MESSAGE_SENSOR = "SENSOR"
    const val MESSAGE_SESSION = "SESSION"

    //subtypes
    const val SUB_TYPE_COMPLETE = "COMPLETE"
    const val SUB_TYPE_START = "START"
    const val SUB_TYPE_SEARCH = "SEARCH"
    const val SUB_TYPE_PAIR = "PAIR"
    const val SUB_TYPE_UN_PAIR = "UN_PAIR"
    const val SUB_TYPE_SWING = "SWING"
    const val SUB_TYPE_BALL_RELEASE = "BALL_RELEASE"
    const val SUB_TYPE_PRE_RELEASE = "PRE_RELEASE"

    //status types
    const val STATUS_INIT = "INIT"
    const val STATUS_SUCCESS = "SUCCESS"
    const val STATUS_INFO = "INFO"
    const val STATUS_FAIL = "SEARCH"

    //sender/receiver type
    const val LANE_APP_LANE_1 = "native-lane-1"
    const val WEB_APP_LANE_1 = "webapp-lane-1"

    //JS-Bridge constants
    const val JS_BRIDGE_NAME = "JSBridge"


}
