package com.stancebeam.cc_lane.data.bluetooth

data class SwingData(
    val address: String,
    val startTs: Long,
    val faPackets: List<String>,
    val fbPackets: List<String>,
)