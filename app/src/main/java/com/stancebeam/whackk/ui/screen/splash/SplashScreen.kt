package com.stancebeam.cc_lane.ui.screen.splash

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.SpanStyle
import androidx.compose.ui.text.buildAnnotatedString
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.withStyle
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.navigation.NavController

import com.stancebeam.cc_lane.ui.navigation.Destination
import com.stancebeam.cc_lane.ui.theme.AppTheme
import com.stancebeam.whackk.BuildConfig
import kotlinx.coroutines.delay

@Composable
private fun SplashContent() {
    Scaffold(
            containerColor = Color.White
    ) { insets ->
        Box(
            modifier = Modifier
                .fillMaxSize()
                .padding(insets)
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
                )
                .padding(
                    horizontal = 32.dp,
                    vertical = 16.dp,
                ),
            contentAlignment = Alignment.Center,
        ) {
            Column(verticalArrangement = Arrangement.Center,
                horizontalAlignment = Alignment.CenterHorizontally
                ) {

                Text(
                    text = buildAnnotatedString {
                        withStyle(SpanStyle(color = Color(0xFFFF3D54), fontWeight = FontWeight.ExtraBold)) {
                            append("WHA")
                        }
                        withStyle(SpanStyle(color = Color.White, fontWeight = FontWeight.ExtraBold)) {
                            append("KK")
                        }
                    },
                    fontSize = 100.sp,
                    textAlign = TextAlign.Center,
                    letterSpacing = 4.sp
                )
                Spacer(Modifier.height(15.dp))

                Text("Version : ${BuildConfig.VERSION_NAME}", color = Color.White.copy(alpha = 0.6f), fontSize = 16.sp, fontWeight = FontWeight.W400)
            }

        }
    }
}

@Composable
fun SplashScreen(
    navController: NavController,

) {

    LaunchedEffect(null) {
        delay(1500)
        navController.navigate(Destination.HomeNav.route) {
                    popUpTo(Destination.Splash.route) {
                        inclusive = true
                    }
                }

    }
    SplashContent()
}

@Preview(widthDp = 1920, heightDp = 1080)
@Composable
fun SplashPreview() {
    AppTheme {
        SplashContent()
    }
}