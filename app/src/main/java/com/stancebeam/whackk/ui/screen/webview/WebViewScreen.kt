package com.stancebeam.cc_lane.ui.screen.webview

import android.webkit.WebView
import android.webkit.WebViewClient
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Scaffold
import androidx.compose.material3.SnackbarHost
import androidx.compose.material3.SnackbarHostState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalSoftwareKeyboardController
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.viewinterop.AndroidView
import androidx.hilt.navigation.compose.hiltViewModel
import androidx.navigation.NavHostController
import com.stancebeam.cc_lane.ui.composable.LoadingDialog

@Composable
fun WebViewScreen(
    rootNavController: NavHostController,
    viewModel: WebViewViewModel = hiltViewModel()
) {
    val context = LocalContext.current
    val keyboardController = LocalSoftwareKeyboardController.current
    val uiState by viewModel.uiState.collectAsState()
    val hostState = remember {
        SnackbarHostState()
    }
    if (uiState.isLoading) {
        LoadingDialog(
            text = "Loading",
        )
    }
    Scaffold(
        snackbarHost = {
            SnackbarHost(hostState)
        },
    ) { innerPadding ->
        WebViewContent(
            modifier = Modifier
                .fillMaxSize()
                .padding(innerPadding), state = uiState,
            buttonClick = {viewModel.sendMessage()}
        )
    }
}

@Composable
fun WebViewContent(modifier: Modifier, state: WebViewState, buttonClick: ()->Unit = {}) {
    Column(
        modifier = modifier
            .padding(
                horizontal = 24.dp,
                vertical = 16.dp,
            ),
        verticalArrangement = Arrangement.Bottom,
    ) {
        AndroidView(
            factory = { context ->
                WebView(context).apply {
                    settings.javaScriptEnabled = true
                    settings.domStorageEnabled = true
                    webViewClient = WebViewClient()
                    loadUrl(state.url)
                }
            },
            update = { webView ->
                // ✅ This ensures the WebView updates if the URL changes
                if (webView.url != state.url) {
                    webView.loadUrl(state.url)
                }
                webView.reload()
            },
            modifier = modifier
        )
    }
}

@Preview
@Composable
fun WebViewScreenPreview() {
    Scaffold(
        snackbarHost = {

        },
    ) { innerPadding ->
        WebViewContent(
            modifier = Modifier
                .fillMaxSize()
                .padding(innerPadding), state = WebViewState()
        )
    }
}