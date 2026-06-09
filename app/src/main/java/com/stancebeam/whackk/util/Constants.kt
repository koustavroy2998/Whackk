package com.stancebeam.cc_lane.util

import java.util.UUID

const val SB_MFR_ID = 57005 // StanceBeam Manufacturer ID
val DATA_SERVICE_UUID: UUID = UUID.fromString("0000aaaa-0000-0000-0000-000000000010")
val DATA_NOTIFY_CHAR_UUID: UUID = UUID.fromString("0000bbbb-0000-0000-0000-000000000010")
val DATA_NOTIFY_DESC_UUID: UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb")
val DATA_FORMAT_CHAR_UUID: UUID = UUID.fromString("0000cccc-0000-0000-0000-000000000010")
const val WS_PROTOCOL = "ws"
const val WS_HOST = "192.168.2.5"
const val WS_PORT = 8765
const val WS_URL = "$WS_PROTOCOL://$WS_HOST:$WS_PORT"
const  val DEFAULT_SENSOR_NAME = "STANCEBEAM"