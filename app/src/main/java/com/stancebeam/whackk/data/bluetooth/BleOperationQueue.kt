package com.stancebeam.cc_lane.data.bluetooth

import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattDescriptor
import android.util.Log
import kotlinx.coroutines.suspendCancellableCoroutine
import java.util.concurrent.ConcurrentLinkedQueue

class BleOperationQueue(private val address: String) {

    companion object {
        private const val TAG = "BleOperationQueue"
    }

    private val operationQueue = ConcurrentLinkedQueue<BleOperation>()
    private var currentOperation: BleOperation? = null
    private var isProcessing = false

    suspend fun enqueueWrite(
        gatt: BluetoothGatt,
        characteristic: BluetoothGattCharacteristic,
        data: ByteArray
    ): Boolean = suspendCancellableCoroutine { continuation ->
        val operation = WriteOperation(gatt, characteristic, data, continuation)
        enqueue(operation)
    }

    suspend fun enqueueDescriptorWrite(
        gatt: BluetoothGatt,
        descriptor: BluetoothGattDescriptor,
        data: ByteArray
    ): Boolean = suspendCancellableCoroutine { continuation ->
        val operation = DescriptorWriteOperation(gatt, descriptor, data, continuation)
        enqueue(operation)
    }

    private fun enqueue(operation: BleOperation) {
        operationQueue.offer(operation)
        Log.d(TAG, "Operation enqueued for $address, queue size: ${operationQueue.size}")
        processNext()
    }

    private fun processNext() {
        if (isProcessing || operationQueue.isEmpty()) {
            Log.d(TAG, "Cannot process next: isProcessing=$isProcessing, queueEmpty=${operationQueue.isEmpty()}")
            return
        }

        Log.d(TAG, "Processing next operation for $address")
        isProcessing = true
        currentOperation = operationQueue.poll()
        currentOperation?.execute()
    }

    fun onOperationComplete(success: Boolean) {
        Log.d(TAG, "Operation completed for $address: $success")
        currentOperation?.complete(success)
        currentOperation = null
        isProcessing = false
        processNext()
    }

    // ADD THIS METHOD
    fun clear() {
        Log.d(TAG, "Clearing queue for $address")
        operationQueue.clear()
        currentOperation?.complete(false) // Complete current operation with failure
        currentOperation = null
        isProcessing = false
    }

    fun isEmpty(): Boolean = operationQueue.isEmpty() && currentOperation == null

    fun size(): Int = operationQueue.size + if (currentOperation != null) 1 else 0
}
