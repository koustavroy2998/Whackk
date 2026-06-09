package com.stancebeam.cc_lane.data.bluetooth

import android.Manifest
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattDescriptor
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothProfile
import android.bluetooth.BluetoothStatusCodes
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.content.Context
import android.os.Build
import android.os.Handler
import android.os.Looper
import android.util.Log
import androidx.annotation.RequiresPermission
import com.stancebeam.cc_lane.util.DATA_FORMAT_CHAR_UUID
import com.stancebeam.cc_lane.util.DATA_NOTIFY_CHAR_UUID
import com.stancebeam.cc_lane.util.DATA_NOTIFY_DESC_UUID
import com.stancebeam.cc_lane.util.DATA_SERVICE_UUID
import com.stancebeam.cc_lane.util.asStanceBeamDevice
import com.stancebeam.cc_lane.util.getStanceBeamDevice
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.withTimeoutOrNull
import java.util.concurrent.ConcurrentHashMap
import kotlin.math.min
import kotlin.math.pow

class BluetoothController(private val context: Context) {

    companion object {
        private const val TAG = "BluetoothController"
        private const val SERVICE_DISCOVERY_TIMEOUT = 10000L
        private const val CONNECTION_TIMEOUT = 15000L
        private const val MAX_RETRY_ATTEMPTS = 3
        private const val BASE_RETRY_DELAY = 250L
        private const val MAX_RETRY_DELAY = 30000L
        private const val SERVICE_DISCOVERY_DELAY = 600L
    }

    private val bluetoothManager by lazy {
        context.getSystemService(BluetoothManager::class.java)
    }
    private val bluetoothAdapter by lazy {
        bluetoothManager?.adapter
    }
    private val bluetoothLeScanner by lazy {
        bluetoothAdapter?.bluetoothLeScanner
    }

    // State management
    private val _scannedDevices = MutableStateFlow<List<StanceBeamDevice>>(emptyList())
    val scannedDevices = _scannedDevices.asStateFlow()
    private val _isScanning = MutableStateFlow(false)
    val isScanning: StateFlow<Boolean> = _isScanning
    private val _connections = ConcurrentHashMap<String, BluetoothGatt>()
    val connections get() = _connections.toMap()
    private val _connectedDevices = MutableStateFlow<List<StanceBeamDevice>>(emptyList())
    val connectedDevices = _connectedDevices.asStateFlow()

    // Enhanced state management
    private val _serviceDiscoveryState = MutableStateFlow<Map<String, Boolean>>(emptyMap())
    val serviceDiscoveryState = _serviceDiscoveryState.asStateFlow()
    private val _connectionStates = MutableStateFlow<Map<String, DeviceConnectionState>>(emptyMap())
    val connectionStates = _connectionStates.asStateFlow()
    private val connectionAttempts = ConcurrentHashMap<String, Int>()
    private val operationQueues = ConcurrentHashMap<String, BleOperationQueue>()
    private val mainHandler = Handler(Looper.getMainLooper())

    // Connection synchronization
    private val connectionLock = Any()

    // Callbacks
    private var sessionCallback: SessionCallback? = null
    private val sessionCallbacks = ConcurrentHashMap<String, SessionCallback>()

    private val scanCallback = object : ScanCallback() {
        override fun onScanResult(callbackType: Int, result: ScanResult?) {
            super.onScanResult(callbackType, result)
            val sbDevice = result?.getStanceBeamDevice()
            sbDevice?.let { device ->
                _scannedDevices.update { devices ->
                    val alreadyExists = devices.any { it.address == device.address }
                    val alreadyConnected = _connectedDevices.value.any { it.address == device.address }
                    if (alreadyExists || alreadyConnected) {
                        devices
                    } else {
                        devices + device
                    }
                }
            }
        }

        override fun onScanFailed(errorCode: Int) {
            super.onScanFailed(errorCode)
            Log.e(TAG, "Scan failed with error: $errorCode")
            _isScanning.value = false
        }
    }

    private val gattCallback = object : BluetoothGattCallback() {
        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        override fun onConnectionStateChange(gatt: BluetoothGatt?, status: Int, newState: Int) {
            super.onConnectionStateChange(gatt, status, newState)
            if (gatt == null) return

            val address = gatt.device.address
            Log.d(TAG, "onConnectionStateChange: $address - status=$status, newState=$newState")

            BleDebugger.logConnectionState(address, "State: $newState", status)
            sessionCallbacks[address]?.onConnectionStateChange(gatt, status, newState)

            when (newState) {
                BluetoothProfile.STATE_CONNECTED -> {
                    if (status == BluetoothGatt.GATT_SUCCESS) {
                        Log.d(TAG, "onConnectionStateChange: Successfully connected to $address")
                        connectionAttempts[address] = 0
                        updateConnectionState(address, DeviceConnectionState.CONNECTED)

                        // Request connection priority for better performance
                        requestConnectionPriority(gatt)

                        // Wait before discovering services (legacy timing)
                        mainHandler.postDelayed({
                            if (_connections.containsKey(address)) { // Still connected
                                updateConnectionState(address, DeviceConnectionState.DISCOVERING_SERVICES)
                                discoverServices(gatt)
                            }
                        }, SERVICE_DISCOVERY_DELAY)

                        onConnected(gatt)
                    } else {
                        Log.w(TAG, "onConnectionStateChange: Connection failed for $address with status $status")
                        handleConnectionError(gatt, status, address)
                    }
                }
                BluetoothProfile.STATE_DISCONNECTED -> {
                    Log.d(TAG, "onConnectionStateChange: STATE_DISCONNECTED for $address")
                    updateConnectionState(address, DeviceConnectionState.DISCONNECTED)
                    cleanupConnection(address) // Only cleanup here
                    onDisconnected(gatt)
                }
                BluetoothProfile.STATE_CONNECTING -> {
                    Log.d(TAG, "onConnectionStateChange: STATE_CONNECTING for $address")
                    updateConnectionState(address, DeviceConnectionState.CONNECTING)
                }
            }
        }

        override fun onServicesDiscovered(gatt: BluetoothGatt?, status: Int) {
            super.onServicesDiscovered(gatt, status)
            if (gatt == null) return

            val address = gatt.device.address
            Log.d(TAG, "onServicesDiscovered: $address - status=$status")

            if (status == BluetoothGatt.GATT_SUCCESS) {
                _serviceDiscoveryState.update { it + (address to true) }
                updateConnectionState(address, DeviceConnectionState.READY)
                BleDebugger.logConnectionState(address, "Services discovered", status)
            } else {
                updateConnectionState(address, DeviceConnectionState.ERROR)
                Log.e(TAG, "Service discovery failed for $address with status: $status")
            }

            sessionCallbacks[address]?.onServicesDiscovered(gatt, status)
        }

        override fun onCharacteristicWrite(
            gatt: BluetoothGatt?,
            characteristic: BluetoothGattCharacteristic?,
            status: Int
        ) {
            super.onCharacteristicWrite(gatt, characteristic, status)
            if (gatt == null || characteristic == null) return

            val address = gatt.device.address
            Log.d(TAG, "onCharacteristicWrite: $address - status=$status")
            BleDebugger.logOperation(address, "Characteristic Write", status == BluetoothGatt.GATT_SUCCESS)

            operationQueues[address]?.onOperationComplete(status == BluetoothGatt.GATT_SUCCESS)
            sessionCallbacks[address]?.onCharacteristicWrite(gatt, characteristic, status)
        }

        override fun onDescriptorWrite(
            gatt: BluetoothGatt?,
            descriptor: BluetoothGattDescriptor?,
            status: Int
        ) {
            super.onDescriptorWrite(gatt, descriptor, status)
            if (gatt == null || descriptor == null) return

            val address = gatt.device.address
            Log.d(TAG, "onDescriptorWrite: $address - status=$status")
            BleDebugger.logOperation(address, "Descriptor Write", status == BluetoothGatt.GATT_SUCCESS)

            operationQueues[address]?.onOperationComplete(status == BluetoothGatt.GATT_SUCCESS)
            sessionCallbacks[address]?.onDescriptorWrite(gatt, descriptor, status)
        }

        override fun onCharacteristicChanged(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            value: ByteArray
        ) {
            super.onCharacteristicChanged(gatt, characteristic, value)
            sessionCallback?.onCharacteristicChanged(gatt, characteristic, value)
            sessionCallbacks[gatt.device?.address]?.onCharacteristicChanged(gatt, characteristic, value)
        }
    }

    // Connection management methods
    private fun updateConnectionState(address: String, state: DeviceConnectionState) {
        _connectionStates.update { it + (address to state) }
        Log.v(TAG, "Connection state updated: $address -> $state")
    }

    private fun requestConnectionPriority(gatt: BluetoothGatt) {
        try {
            val result = gatt.requestConnectionPriority(BluetoothGatt.CONNECTION_PRIORITY_HIGH)
            Log.d(TAG, "Connection priority request: $result")
        } catch (e: SecurityException) {
            Log.e(TAG, "Failed to request connection priority", e)
        }
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    private fun handleConnectionError(gatt: BluetoothGatt, status: Int, address: String) {
        val attempts = connectionAttempts.getOrDefault(address, 0)
        Log.w(TAG, "handleConnectionError: $address - status=$status, attempts=$attempts")

        when (status) {
            133 -> { // GATT_ERROR - most common
                if (attempts < MAX_RETRY_ATTEMPTS) {
                    connectionAttempts[address] = attempts + 1
                    gatt.close()

                    val delay = min(
                        BASE_RETRY_DELAY * (2.0.pow(attempts)).toLong(),
                        MAX_RETRY_DELAY
                    )

                    Log.d(TAG, "Scheduling reconnection attempt ${attempts + 1} for $address in ${delay}ms")
                    mainHandler.postDelayed({
                        connectDevice(address)
                    }, delay)
                } else {
                    Log.e(TAG, "Max retry attempts reached for $address")
                    updateConnectionState(address, DeviceConnectionState.ERROR)
                    gatt.close()
                    cleanupConnection(address)
                }
            }
            8 -> { // Connection timeout
                Log.w(TAG, "Connection timeout for $address")
                updateConnectionState(address, DeviceConnectionState.ERROR)
                gatt.close()
                cleanupConnection(address)
            }
            else -> {
                Log.e(TAG, "Connection failed for $address with status: $status")
                updateConnectionState(address, DeviceConnectionState.ERROR)
                gatt.close()
                cleanupConnection(address)
            }
        }
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    private fun cleanupConnection(address: String) {
        Log.d(TAG, "cleanupConnection: starting for $address")

        // Synchronize cleanup to prevent race conditions
        synchronized(connectionLock) {
            val gatt = _connections.remove(address)
            if (gatt != null) {
                Log.d(TAG, "cleanupConnection: closing GATT for $address")
                try {
                    gatt.close()
                } catch (e: Exception) {
                    Log.e(TAG, "Error closing GATT for $address", e)
                }
            } else {
                Log.d(TAG, "cleanupConnection: no GATT found for $address (already cleaned up)")
            }

            operationQueues.clear()
            connectionAttempts.remove(address)
            _serviceDiscoveryState.update { it - address }
            _connectionStates.update { it - address }
        }

        Log.d(TAG, "cleanupConnection: completed for $address")
    }

    private fun validateConnection(address: String): BluetoothGatt? {
        val gatt = _connections[address]
        if (gatt == null) {
            Log.w(TAG, "No GATT connection for $address")
            return null
        }

        if (serviceDiscoveryState.value[address] != true) {
            Log.w(TAG, "Services not discovered for $address")
            return null
        }

        return gatt
    }

    // Public API methods
    fun setSessionCallback(callback: SessionCallback) {
        sessionCallback = callback
    }

    fun setSessionCallback(address: String, callback: SessionCallback) {
        sessionCallbacks[address] = callback
    }

    fun removeSessionCallback(address: String) {
        sessionCallbacks.remove(address)
    }

    private fun discoverServices(gatt: BluetoothGatt): Boolean {
        return try {
            val result = gatt.discoverServices()
            Log.d(TAG, "discoverServices: result = $result")
            result
        } catch (exception: SecurityException) {
            Log.e(TAG, "discoverServices: ", exception)
            false
        }
    }

    private fun onConnected(gatt: BluetoothGatt) {
        gatt.device?.let { device ->
            _scannedDevices.update { devices ->
                devices.filter { it.address != device.address }
            }
            _connectedDevices.update { devices ->
                val alreadyExists = devices.any { it.address == device.address }
                if (alreadyExists) {
                    devices
                } else {
                    devices + device.asStanceBeamDevice()
                }
            }
        }
    }

    private fun onDisconnected(gatt: BluetoothGatt) {
        gatt.device?.let { device ->
            _connectedDevices.update { devices ->
                devices.filter { it.address != device.address }
            }
        }
    }

    fun isBluetoothEnabled(): Boolean {
        return bluetoothAdapter?.isEnabled == true
    }

    fun startScan() {
        _scannedDevices.update { emptyList() }
        try {
            bluetoothLeScanner?.startScan(scanCallback)
            _isScanning.value = true
        } catch (exception: SecurityException) {
            Log.e(TAG, "startScan: ", exception)
        }
    }

    fun stopScan() {
        try {
            bluetoothLeScanner?.stopScan(scanCallback)
            _isScanning.value = false
        } catch (exception: SecurityException) {
            Log.e(TAG, "stopScan: ", exception)
        }
    }
    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    fun connectDevice(address: String) {
        Log.d(TAG, "connectDevice: initiated for $address")

        synchronized(connectionLock) {
            if (_connections.containsKey(address)) {
                val currentState = _connectionStates.value[address]
                Log.d(TAG, "connectDevice: address already in _connections with state: $currentState")

                // If we're already connected or connecting, don't start a new connection
                if (currentState in listOf(
                        DeviceConnectionState.CONNECTED,
                        DeviceConnectionState.CONNECTING,
                        DeviceConnectionState.DISCOVERING_SERVICES,
                        DeviceConnectionState.READY
                    )) {
                    Log.d(TAG, "connectDevice: skipping - already connected/connecting")
                    return
                }

                // If in error state or unknown state, clean up first
                Log.d(TAG, "connectDevice: cleaning up previous connection in state: $currentState")
                cleanupConnection(address)
            }

            updateConnectionState(address, DeviceConnectionState.CONNECTING)

            try {
                val device = bluetoothAdapter?.getRemoteDevice(address)
                val bluetoothGatt = device?.connectGatt(context, false, gattCallback)
                bluetoothGatt?.let { gatt ->
                    _connections[address] = gatt
                    Log.d(TAG, "connectDevice: GATT connection initiated for $address")
                } ?: run {
                    Log.e(TAG, "connectDevice: failed to create GATT connection for $address")
                    updateConnectionState(address, DeviceConnectionState.ERROR)
                }
            } catch (exception: SecurityException) {
                Log.e(TAG, "connectDevice: SecurityException", exception)
                updateConnectionState(address, DeviceConnectionState.ERROR)
            }
        }
    }

    fun disconnectDevice(address: String) {
        Log.d(TAG, "disconnectDevice: initiated for $address")

        synchronized(connectionLock) {
            val gatt = _connections[address]
            if (gatt == null) {
                Log.d(TAG, "disconnectDevice: address not in _connections - $address")
                return
            }

            try {
                Log.d(TAG, "disconnectDevice: calling disconnect() for $address")
                gatt.disconnect()
                // Don't call cleanupConnection here - wait for callback
            } catch (exception: SecurityException) {
                Log.e(TAG, "disconnectDevice: SecurityException", exception)
                // Force cleanup if disconnect fails
                cleanupConnection(address)
            } catch (exception: Exception) {
                Log.e(TAG, "disconnectDevice: Exception", exception)
                // Force cleanup if disconnect fails
                cleanupConnection(address)
            }
        }
    }

    fun disconnectAllDevices() {
        Log.d(TAG, "disconnectAllDevices: initiated")
        val addressesToDisconnect = _connections.keys.toList()
        addressesToDisconnect.forEach { address ->
            disconnectDevice(address)
        }
    }

    // Enhanced write methods with queuing
    suspend fun writeDataFormatQueued(address: String, value: ByteArray): Boolean {
        val gatt = validateConnection(address) ?: return false
        val queue = operationQueues.getOrPut(address) { BleOperationQueue(address) }

        return gatt.getService(DATA_SERVICE_UUID)
            ?.getCharacteristic(DATA_FORMAT_CHAR_UUID)
            ?.let { characteristic ->
                queue.enqueueWrite(gatt, characteristic, value)
            } ?: false
    }

    suspend fun setDataNotificationQueued(address: String, enabled: Boolean): Boolean {
        val gatt = validateConnection(address) ?: return false
        val queue = operationQueues.getOrPut(address) { BleOperationQueue(address) }

        return try {
            val service = gatt.getService(DATA_SERVICE_UUID)
            val characteristic = service?.getCharacteristic(DATA_NOTIFY_CHAR_UUID)
            val descriptor = characteristic?.getDescriptor(DATA_NOTIFY_DESC_UUID)

            if (service == null || characteristic == null || descriptor == null) {
                Log.e(TAG, "Required service/characteristic/descriptor not found")
                return false
            }

            val res = gatt.setCharacteristicNotification(characteristic, enabled)
            Log.d(TAG, "setDataNotificationQueued: gatt.setCharacteristicNotification = $res")

            val descriptorValue = if (enabled) {
                BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
            } else {
                BluetoothGattDescriptor.DISABLE_NOTIFICATION_VALUE
            }

            queue.enqueueDescriptorWrite(gatt, descriptor, descriptorValue)
        } catch (e: SecurityException) {
            Log.e(TAG, "setDataNotificationQueued: ", e)
            false
        }
    }

    // Legacy methods for backward compatibility
    fun writeDataFormat(gatt: BluetoothGatt, value: ByteArray): Boolean {
        return try {
            gatt.getService(DATA_SERVICE_UUID)
                ?.getCharacteristic(DATA_FORMAT_CHAR_UUID)
                ?.let { characteristic ->
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                        val result = gatt.writeCharacteristic(
                            characteristic, value, BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
                        )
                        result == BluetoothStatusCodes.SUCCESS
                    } else {
                        val result = characteristic.setValue(value)
                        Log.d(TAG, "writeDataFormat: characteristic.setValue: result = $result")
                        result && gatt.writeCharacteristic(characteristic)
                    }
                } ?: false
        } catch (e: SecurityException) {
            Log.e(TAG, "writeDataFormat: ", e)
            false
        }
    }

    fun setDataNotification(gatt: BluetoothGatt, enabled: Boolean): Boolean {
        return try {
            val service = gatt.getService(DATA_SERVICE_UUID)
            val characteristic = service?.getCharacteristic(DATA_NOTIFY_CHAR_UUID)
            val descriptor = characteristic?.getDescriptor(DATA_NOTIFY_DESC_UUID)

            if (service == null || characteristic == null || descriptor == null) {
                return false
            }

            val res = gatt.setCharacteristicNotification(characteristic, enabled)
            Log.d(TAG, "setDataNotification: gatt.setCharacteristicNotification = $res")
            val descriptorValue = if (enabled) {
                BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
            } else {
                BluetoothGattDescriptor.DISABLE_NOTIFICATION_VALUE
            }
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                val result = gatt.writeDescriptor(descriptor, descriptorValue)
                result == BluetoothStatusCodes.SUCCESS
            } else {
                val result = descriptor.setValue(descriptorValue)
                Log.d(TAG, "setDataNotification: descriptor.setValue = $result")
                result && gatt.writeDescriptor(descriptor)
            }
        } catch (e: SecurityException) {
            Log.e(TAG, "setDataNotification: ", e)
            false
        }
    }

    // Utility methods
    suspend fun waitForDeviceReady(address: String, timeoutMs: Long = SERVICE_DISCOVERY_TIMEOUT): Boolean {
        return withTimeoutOrNull(timeoutMs) {
            while (_connectionStates.value[address] != DeviceConnectionState.READY) {
                delay(100)
            }
            true
        } ?: false
    }

    fun isDeviceReady(address: String): Boolean {
        return _connectionStates.value[address] == DeviceConnectionState.READY
    }

    fun getConnectionState(address: String): DeviceConnectionState {
        return _connectionStates.value[address] ?: DeviceConnectionState.DISCONNECTED
    }

    fun getConnectedAddresses(): List<String> {
        return _connections.keys.toList()
    }

    fun isConnected(address: String): Boolean {
        return _connections.containsKey(address) &&
                _connectionStates.value[address] in listOf(
            DeviceConnectionState.CONNECTED,
            DeviceConnectionState.DISCOVERING_SERVICES,
            DeviceConnectionState.READY
        )
    }

    fun release() {
        Log.d(TAG, "release: starting cleanup")
        stopScan()
        disconnectAllDevices()

        // Wait a bit for disconnections to complete
        mainHandler.postDelayed({
            sessionCallbacks.clear()
            operationQueues.clear()
            operationQueues.clear()
            connectionAttempts.clear()
            _serviceDiscoveryState.value = emptyMap()
            _connectionStates.value = emptyMap()
            Log.d(TAG, "release: cleanup completed")
        }, 1000)
    }
}

// Supporting enums and classes


// Debug utility
object BleDebugger {
    private const val TAG = "BleDebugger"

    fun logConnectionState(address: String, state: String, status: Int) {
        Log.d(TAG, "[$address] Connection: $state (status: $status)")
    }

    fun logOperation(address: String, operation: String, success: Boolean) {
        Log.d(TAG, "[$address] Operation: $operation - ${if (success) "SUCCESS" else "FAILED"}")
    }

    fun logSessionState(address: String, state: String) {
        Log.d(TAG, "[$address] Session: $state")
    }
}
