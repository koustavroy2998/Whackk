package com.stancebeam.cc_lane.util

import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import android.bluetooth.le.ScanResult
import android.util.Patterns
import androidx.core.util.forEach
import com.stancebeam.cc_lane.data.bluetooth.StanceBeamDevice
import com.stancebeam.cc_lane.data.entity.Striker

fun String.isEmail(): Boolean {
    return Patterns.EMAIL_ADDRESS.matcher(this)
        .matches()
}

fun String.isNotEmail(): Boolean {
    return isEmail().not()
}

@SuppressLint("MissingPermission")
fun BluetoothDevice.asStanceBeamDevice(): StanceBeamDevice {
    return StanceBeamDevice(
        name = name,
        address = address,
    )
}

@SuppressLint("MissingPermission")
fun BluetoothDevice.asStriker(): Striker {
    return Striker(
        address = address,
        name = name,
    )
}

fun ScanResult.getStanceBeamDevice(): StanceBeamDevice? {
    scanRecord?.manufacturerSpecificData?.forEach { key, _ ->
        if (key == SB_MFR_ID) {
            return device.asStanceBeamDevice()
        }
    }
    return null
}