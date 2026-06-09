package com.stancebeam.cc_lane.data.bluetooth

import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow

enum class DeviceConnectionState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    DISCOVERING_SERVICES,
    READY,
    ERROR
}

class ConnectionStateManager {
    private val _deviceStates = MutableStateFlow<Map<String, DeviceConnectionState>>(emptyMap())
    val deviceStates: StateFlow<Map<String, DeviceConnectionState>> = _deviceStates.asStateFlow()

    fun updateState(address: String, state: DeviceConnectionState) {
        _deviceStates.value = _deviceStates.value + (address to state)
    }

    fun getState(address: String): DeviceConnectionState {
        return _deviceStates.value[address] ?: DeviceConnectionState.DISCONNECTED
    }

    fun removeDevice(address: String) {
        _deviceStates.value = _deviceStates.value - address
    }

    fun isReady(address: String): Boolean {
        return getState(address) == DeviceConnectionState.READY
    }
}
