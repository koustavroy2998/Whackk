package com.stancebeam.cc_lane.ui.composable.webview

import android.util.Log
import android.widget.Toast
import androidx.compose.foundation.background
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.shadow
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import com.airbnb.lottie.compose.*

@Composable
fun LoadingOverlay(showLoader: Boolean) {
    if (showLoader) {
        Box(
            modifier = Modifier
                .fillMaxSize()
                .background(
                    brush = Brush.linearGradient(
                        colorStops = arrayOf(
                            0.0f to Color(0xFFFF3D54),
                            0.45f to Color(0xFF000000),
                            1.0f to Color(0xFF000000)
                        ),
                        start = Offset(0f, 0f),
                        end = Offset(Float.POSITIVE_INFINITY, Float.POSITIVE_INFINITY)
                    )
                ),
            contentAlignment = Alignment.Center
        ) {
            val composition by rememberLottieComposition(
                LottieCompositionSpec.Asset("bat_ball.lottie")
            )
            val progress by animateLottieCompositionAsState(
                composition = composition,
                iterations = LottieConstants.IterateForever,
                isPlaying = true
            )

            Column(horizontalAlignment = Alignment.CenterHorizontally) {
                LottieAnimation(
                    composition = composition,
                    progress = { progress },
                    modifier = Modifier
                        .size(150.dp)
                        .shadow(10.dp, CircleShape)
                )

                Spacer(Modifier.height(20.dp))

                Text(
                    "Loading...",
                    color = Color.White,
                    style = MaterialTheme.typography.titleMedium.copy(
                        fontWeight = FontWeight.Medium
                    )
                )
            }
        }
    }
}

@Composable
fun DownloadProgressDialog(
    showDownloadDialog: Boolean,
    downloadFileName: String,
    downloadProgress: Int,
    onDismiss: () -> Unit
) {
    if (showDownloadDialog) {
        AlertDialog(
            onDismissRequest = onDismiss,
            title = {
                Text(
                    "Downloading File",
                    style = MaterialTheme.typography.titleMedium
                )
            },
            text = {
                Column {
                    Text(
                        downloadFileName,
                        style = MaterialTheme.typography.bodyMedium,
                        maxLines = 2,
                        overflow = TextOverflow.Ellipsis
                    )

                    Spacer(Modifier.height(16.dp))

                    LinearProgressIndicator(
                        progress = { downloadProgress / 100f },
                        modifier = Modifier.fillMaxWidth(),
                        color = Color(0xFF1A2148),
                        trackColor = ProgressIndicatorDefaults.linearTrackColor,
                        strokeCap = ProgressIndicatorDefaults.LinearStrokeCap,
                    )

                    Spacer(Modifier.height(8.dp))

                    Text(
                        when {
                            downloadProgress == 0 -> "Processing download..."
                            downloadProgress > 0 -> "$downloadProgress%"
                            else -> "Converting data..."
                        },
                        style = MaterialTheme.typography.bodySmall,
                        color = Color.Gray
                    )
                }
            },
            confirmButton = {
                Button(
                    onClick = onDismiss,
                    colors = ButtonDefaults.buttonColors(
                        containerColor = Color(0xFF1A2148),
                        contentColor = Color.White
                    )
                ) {
                    Text("Hide")
                }
            }
        )
    }
}

@Composable
fun FolderSelectionDialog(
    showFolderDialog: Boolean,
    onSelectFolder: () -> Unit,
    onDismiss: () -> Unit
) {
    if (showFolderDialog) {
        AlertDialog(
            onDismissRequest = onDismiss,
            title = { Text("Select LaneConfig Folder") },
            text = { Text("Please select the LaneConfig folder to load the URL.") },
            confirmButton = {
                Button(
                    onClick = onSelectFolder,
                    colors = ButtonDefaults.buttonColors(
                        containerColor = Color(0xFF1A2148),
                        contentColor = Color.White
                    )
                ) {
                    Text("Select Folder")
                }
            },
            dismissButton = {
                Button(
                    onClick = onDismiss,
                    colors = ButtonDefaults.buttonColors(
                        containerColor = Color(0xFFF44336),
                        contentColor = Color.White
                    )
                ) {
                    Text("Cancel")
                }
            }
        )
    }
}

@Composable
fun SecretLongPressZone(
    onLongPress: () -> Unit,
    modifier: Modifier = Modifier
) {
    val context = LocalContext.current
    
    Box(
        modifier = modifier
            .fillMaxSize()
            .padding(16.dp),
        contentAlignment = Alignment.BottomEnd
    ) {
        Box(
            modifier = Modifier
                .size(100.dp)
                .background(Color.Transparent)
                .pointerInput(Unit) {
                    detectTapGestures(
                        onLongPress = {
                            onLongPress()
                            Log.d("WebViewContent2", "🕵️ Secret 3s long press triggered")
                            Toast.makeText(
                                context,
                                "Secret long press detected",
                                Toast.LENGTH_SHORT
                            ).show()
                        }
                    )
                }
        )
    }
}
