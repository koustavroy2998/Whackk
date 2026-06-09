package com.stancebeam.cc_lane.data.bluetooth

import com.stancebeam.cc_lane.data.model.DeviceConnection
import com.stancebeam.cc_lane.data.repository.StrikerRepository
import kotlinx.coroutines.flow.combine
import kotlinx.coroutines.flow.map
import javax.inject.Inject

class ConnectionController @Inject constructor(
    bluetoothController: BluetoothController,
    strikerRepository: StrikerRepository,
) {

    val connectedDevices = bluetoothController.connectedDevices
    val playersMap = strikerRepository.getStrikers()
        .map {
            it.associate { striker ->
                striker.striker.address to striker.player
            }
        }
    val deviceConnections = combine(connectedDevices, playersMap) { devices, players ->
        devices.map { device ->
            DeviceConnection(
                device = device,
                player = players[device.address],
            )
        }
    }
}