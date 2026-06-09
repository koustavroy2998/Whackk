package com.stancebeam.cc_lane.data.util

import android.Manifest
import android.content.Context
import android.content.pm.PackageManager
import android.os.Build
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update

class PermissionController(private val context: Context) {

    private val _requiredPermissions = mutableListOf<String>()
    val requiredPermissions get() = _requiredPermissions.toTypedArray()
    private val _hasPermissions = MutableStateFlow(false)
    val hasPermissions = _hasPermissions.asStateFlow()

    init {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            _requiredPermissions.add(Manifest.permission.BLUETOOTH_SCAN)
            _requiredPermissions.add(Manifest.permission.BLUETOOTH_CONNECT)
        } else {
            _requiredPermissions.add(Manifest.permission.ACCESS_FINE_LOCATION)
            _requiredPermissions.add(Manifest.permission.ACCESS_COARSE_LOCATION)
        }
        _hasPermissions.update {
            _requiredPermissions.all {
                context.checkSelfPermission(it) == PackageManager.PERMISSION_GRANTED
            }
        }
    }

    fun onPermissionsResult(result: Map<String, Boolean>) {
        _hasPermissions.update {
            result.values.all {
                it
            }
        }
    }
}