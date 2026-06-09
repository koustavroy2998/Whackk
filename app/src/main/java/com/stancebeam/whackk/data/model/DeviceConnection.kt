package com.stancebeam.cc_lane.data.model

import com.stancebeam.cc_lane.data.bluetooth.StanceBeamDevice
import com.stancebeam.cc_lane.data.entity.Player

data class DeviceConnection(
    val device: StanceBeamDevice,
    val player: Player?,
    val loading: Boolean = false,
)