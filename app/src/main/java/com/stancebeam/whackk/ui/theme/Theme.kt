package com.stancebeam.cc_lane.ui.theme

import androidx.compose.foundation.background
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color

private val lightScheme = lightColorScheme()
private val darkScheme = darkColorScheme()

@Composable
fun AppTheme(
    darkTheme: Boolean = isSystemInDarkTheme(),
    content: @Composable () -> Unit,
) {
    val colorScheme = if (darkTheme) darkScheme else lightScheme

    // Wrap content in a full-screen Box with gradient background to prevent white flash
    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(
                Brush.linearGradient(
                    colors = listOf(Color(0xFF1A2148), Color(0xFF0D1024)), // your gradient
                    start = Offset(0f, 0f),
                    end = Offset.Infinite
                )
            )
    ) {
        MaterialTheme(
            colorScheme = colorScheme,
            content = content
        )
    }
}
