package com.stancebeam.cc_lane.data.bluetooth

import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattDescriptor

interface SessionCallback {

    fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {}

    fun onCharacteristicWrite(
        gatt: BluetoothGatt,
        characteristic: BluetoothGattCharacteristic,
        status: Int,
    ) {
    }

    fun onDescriptorWrite(
        gatt: BluetoothGatt,
        descriptor: BluetoothGattDescriptor,
        status: Int,
    ) {
    }

    fun onCharacteristicChanged(
        gatt: BluetoothGatt,
        characteristic: BluetoothGattCharacteristic,
        value: ByteArray,
    ) {
    }

    fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {}  // This was missing!
}