package com.stancebeam.cc_lane.data.bluetooth

import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattDescriptor
import android.bluetooth.BluetoothStatusCodes
import android.os.Build
import android.util.Log
import kotlin.coroutines.Continuation
import kotlin.coroutines.resume

// Abstract base class for BLE operations
abstract class BleOperation {
    abstract fun execute()
    abstract fun complete(success: Boolean)
}

// Write characteristic operation
@Suppress("DEPRECATION")
class WriteOperation(
    private val gatt: BluetoothGatt,
    private val characteristic: BluetoothGattCharacteristic,
    private val data: ByteArray,
    private val continuation: Continuation<Boolean>
) : BleOperation() {

    companion object {
        private const val TAG = "WriteOperation"
    }

    override fun execute() {
        try {
            val success = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                val result = gatt.writeCharacteristic(
                    characteristic,
                    data,
                    BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
                )
                result == BluetoothStatusCodes.SUCCESS
            } else {
                characteristic.value = data
                gatt.writeCharacteristic(characteristic)
            }

            Log.d(TAG, "Write operation initiated: $success")
            if (!success) {
                complete(false)
            }
            // If success, wait for callback to call complete()

        } catch (e: SecurityException) {
            Log.e(TAG, "Security exception in write operation", e)
            complete(false)
        }
    }

    override fun complete(success: Boolean) {
        continuation.resume(success)
    }
}

// Write descriptor operation
@Suppress("DEPRECATION")
class DescriptorWriteOperation(
    private val gatt: BluetoothGatt,
    private val descriptor: BluetoothGattDescriptor,
    private val data: ByteArray,
    private val continuation: Continuation<Boolean>
) : BleOperation() {

    companion object {
        private const val TAG = "DescriptorWriteOperation"
    }

    override fun execute() {
        try {
            val success = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                val result = gatt.writeDescriptor(descriptor, data)
                result == BluetoothStatusCodes.SUCCESS
            } else {
                descriptor.value = data
                gatt.writeDescriptor(descriptor)
            }

            Log.d(TAG, "Descriptor write operation initiated: $success")
            if (!success) {
                complete(false)
            }
            // If success, wait for callback to call complete()

        } catch (e: SecurityException) {
            Log.e(TAG, "Security exception in descriptor write operation", e)
            complete(false)
        }
    }

    override fun complete(success: Boolean) {
        continuation.resume(success)
    }
}

// Queue manager for sequential BLE operations

