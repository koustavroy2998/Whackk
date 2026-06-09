package com.stancebeam.whackk.ui

import android.app.AlertDialog
import android.bluetooth.BluetoothAdapter
import android.content.ActivityNotFoundException
import android.content.Intent
import android.os.Build
import android.os.Bundle
import android.view.View
import android.view.WindowInsets
import android.view.WindowInsetsController
import android.view.WindowManager
import android.widget.FrameLayout
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.contract.ActivityResultContracts
import androidx.activity.viewModels
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.platform.ComposeView
import androidx.core.view.WindowCompat
import androidx.lifecycle.lifecycleScope
import com.google.android.play.core.appupdate.AppUpdateManager
import com.google.android.play.core.appupdate.AppUpdateManagerFactory
import com.google.android.play.core.install.InstallStateUpdatedListener
import com.google.android.play.core.install.model.InstallStatus
import com.google.android.play.core.install.model.UpdateAvailability

import com.stancebeam.cc_lane.ui.navigation.AppNavigation
import com.stancebeam.cc_lane.ui.navigation.Destination
import com.stancebeam.cc_lane.ui.theme.AppTheme
import dagger.hilt.android.AndroidEntryPoint
import kotlinx.coroutines.flow.collectLatest
import kotlinx.coroutines.launch
import androidx.core.net.toUri
import com.stancebeam.cc_lane.ui.MainViewModel
import com.stancebeam.whackk.R


@AndroidEntryPoint
class MainActivity : ComponentActivity() {

    private val viewModel: MainViewModel by viewModels()
    private val permissionsLauncher = registerForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ) {
        viewModel.onPermissionsResult(it)
    }
    private val bluetoothLauncher = registerForActivityResult(
        ActivityResultContracts.StartActivityForResult()
    ) {}

    private val UPDATE_REQUEST_CODE = 101
    private lateinit var appUpdateManager: AppUpdateManager
    private val listener = InstallStateUpdatedListener { state ->
        if (state.installStatus() == InstallStatus.DOWNLOADED) {
            showCompleteUpdateDialog()
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE)
        setContentView(R.layout.main_activity)
        enableEdgeToEdge()
        requestPermissions()

        lifecycleScope.launch {
            viewModel.hasPermissions.collectLatest {
                if (it) enableBluetooth()
            }
        }

        val container = findViewById<FrameLayout>(R.id.compose_container)
        val composeView = ComposeView(this).apply {
            setContent {
                val darkTheme by viewModel.darkTheme.collectAsState(initial = null)
                AppTheme(
                    darkTheme = darkTheme ?: isSystemInDarkTheme()
                ) {
                    AppNavigation(Destination.Splash.route)
                }
            }
        }
        container.addView(composeView)
        hideSystemUI()

        // ✅ Initialize app update manager
        appUpdateManager = AppUpdateManagerFactory.create(this)
        appUpdateManager.registerListener(listener)

//        viewModel.sendLaneDataToGoogleSheet(
//            this,
//            laneNumber = "2",
//            appVersion = BuildConfig.VERSION_NAME,
//            testerEmail = "houston_lane2@example.com"
//        )

        // ✅ Check for app update every launch
        checkForAppUpdate()
    }

    private fun requestPermissions() {
        permissionsLauncher.launch(viewModel.requiredPermissions)
    }

    private fun enableBluetooth() {
        if (viewModel.isBluetoothEnabled) return
        val intent = Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE)
        bluetoothLauncher.launch(intent)
    }




    private fun hideSystemUI() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            WindowCompat.setDecorFitsSystemWindows(window, false)
            window.insetsController?.let { controller ->
                controller.hide(WindowInsets.Type.navigationBars() or WindowInsets.Type.statusBars())
                controller.systemBarsBehavior =
                    WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
            }
        } else {
            @Suppress("DEPRECATION")
            window.decorView.systemUiVisibility =
                (View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                        or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                        or View.SYSTEM_UI_FLAG_FULLSCREEN
                        or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                        or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                        or View.SYSTEM_UI_FLAG_LAYOUT_STABLE)
        }
    }

    // 🔄 In-App Update check (Android 14–safe)
    private fun checkForAppUpdate() {
        val appUpdateInfoTask = appUpdateManager.appUpdateInfo
        appUpdateInfoTask.addOnSuccessListener { appUpdateInfo ->
            if (appUpdateInfo.updateAvailability() == UpdateAvailability.UPDATE_AVAILABLE) {
                AlertDialog.Builder(this)
                    .setTitle("Update Available")
                    .setMessage("A new version of the app is available. Please update to continue.")
                    .setPositiveButton("Update") { _, _ ->
                        openPlayStoreForUpdate()
                    }
                    .setNegativeButton("Later", null)
                    .show()
            }
        }
    }


    private fun showCompleteUpdateDialog() {
        AlertDialog.Builder(this)
            .setTitle("Update Ready")
            .setMessage("The new version has been downloaded. Restart now to update?")
            .setPositiveButton("Restart") { _, _ ->
                appUpdateManager.completeUpdate() // ✅ This auto-restarts app with new version
            }
            .setCancelable(false)
            .show()
    }

    override fun onDestroy() {
        appUpdateManager.unregisterListener(listener)
        super.onDestroy()
    }

    private fun openPlayStoreForUpdate() {
        val appPackageName = packageName
        val intent = Intent(
            Intent.ACTION_VIEW,
            "market://details?id=$appPackageName".toUri()
        ).apply {
            // Ensure the Play Store app handles it when available
            setPackage("com.android.vending")
            addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TOP)
        }
        try {
            startActivity(intent)
        } catch (e: ActivityNotFoundException) {
            startActivity(
                Intent(
                    Intent.ACTION_VIEW,
                    "https://play.google.com/store/apps/details?id=$appPackageName".toUri()
                ).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TOP)
            )
        }
    }


    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode == UPDATE_REQUEST_CODE && resultCode != RESULT_OK) {
            Toast.makeText(this, "Update canceled", Toast.LENGTH_SHORT).show()
        }
    }
}
