package com.stancebeam.cc_lane.ui.composable.webview

import android.annotation.SuppressLint
import android.webkit.WebView
import android.widget.FrameLayout
import android.widget.Toast
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.ComposeView
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.viewinterop.AndroidView
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat

import android.util.Log
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.hilt.navigation.compose.hiltViewModel
import com.stancebeam.cc_lane.ui.screen.home.HomeViewModel
import dagger.hilt.android.lifecycle.HiltViewModel
import javax.inject.Inject

@SuppressLint("ClickableViewAccessibility", "JavascriptInterface")
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun WebViewContent2(
    modifier: Modifier = Modifier,
    defaultUrl: String,
    viewModel: HomeViewModel = hiltViewModel(),
    jsInterfaces: WebViewJSInterfaces = hiltViewModel<WebViewDependencyProvider>().jsInterfaces,
    downloadHandlerFactory: WebViewDownloadHandler.Factory = hiltViewModel<WebViewDependencyProvider>().downloadHandlerFactory
) {
    val context = LocalContext.current
    var webView by remember { mutableStateOf<WebView?>(null) }
    var showFolderDialog by remember { mutableStateOf(false) }

    val urlFromJson by viewModel.urlState.collectAsState()
    val folderUri by viewModel.folderUri.collectAsState()

    // State variables
    var urlToLoad by remember { mutableStateOf<String?>(null) }
    var pageDone by remember { mutableStateOf(false) }
    var contentReady by remember { mutableStateOf(false) }
    var showLoader by remember { mutableStateOf(true) }
    var progressComplete by remember { mutableStateOf(false) }
    var showDownloadDialog by remember { mutableStateOf(false) }
    var downloadProgress by remember { mutableStateOf(0) }
    var downloadFileName by remember { mutableStateOf("") }

    // Initialize handlers and interfaces
    val jsBridgeController = remember { viewModel.getJSBridgeController() }
    val jsBridgeInterface = remember { viewModel.getJSBridgeInterface() }

    val downloadHandler = remember { downloadHandlerFactory.create() }

    // Set download handler callbacks
    LaunchedEffect(Unit) {
        downloadHandler.setCallbacks(
            onDownloadStart = { filename ->
                downloadFileName = filename
                showDownloadDialog = true
                downloadProgress = 0
            },
            onDownloadComplete = {
                showDownloadDialog = false
            },
            onDownloadError = { error ->
                showDownloadDialog = false
                Toast.makeText(context, error, Toast.LENGTH_LONG).show()
            }
        )
    }

    val contentReadyInterface = remember {
        jsInterfaces.createContentReadyInterface(
            onContentReady = { contentReady = true },
            onReactAppReady = { contentReady = true }
        )
    }

    val blobDownloadInterface = remember {
        jsInterfaces.createBlobDownloadInterface(
            onDownloadComplete = { showDownloadDialog = false },
            onDownloadError = { error ->
                showDownloadDialog = false
                Toast.makeText(context, error, Toast.LENGTH_LONG).show()
            }
        )
    }

    val pickFolderLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.OpenDocumentTree()
    ) { uri ->
        uri?.let { viewModel.saveFolderUri(it) }
        showFolderDialog = false
        showLoader = false
        Log.d("WebViewContent2", "📂 Folder selected: $uri")
    }

    // URL logic
    LaunchedEffect(folderUri, urlFromJson) {
        urlToLoad = if (folderUri != null && !urlFromJson.isNullOrEmpty()) {
            urlFromJson
        } else defaultUrl
        pageDone = false
        contentReady = false
        progressComplete = false
        showLoader = true
    }

    Box(modifier = modifier.fillMaxSize()) {
        AndroidView(
            factory = { ctx ->
                FrameLayout(ctx).apply {
                    layoutParams = FrameLayout.LayoutParams(
                        FrameLayout.LayoutParams.MATCH_PARENT,
                        FrameLayout.LayoutParams.MATCH_PARENT
                    )

                    // Handle keyboard insets
                    ViewCompat.setOnApplyWindowInsetsListener(this) { view, insets ->
                        val imeVisible = insets.isVisible(WindowInsetsCompat.Type.ime())
                        val imeHeight = insets.getInsets(WindowInsetsCompat.Type.ime()).bottom

                        if (imeVisible) {
                            Log.d("WebViewContent2", "⌨️ Keyboard visible with height: $imeHeight")

                            webView?.evaluateJavascript("""
                                (function() {
                                    const activeElement = document.activeElement;
                                    if (activeElement && (activeElement.tagName === 'INPUT' || activeElement.tagName === 'TEXTAREA')) {
                                        activeElement.scrollIntoView({
                                            behavior: 'instant',
                                            block: 'center',
                                            inline: 'nearest'
                                        });
                                        
                                        const rect = activeElement.getBoundingClientRect();
                                        const keyboardHeight = $imeHeight;
                                        const windowHeight = window.innerHeight;
                                        const availableHeight = windowHeight - keyboardHeight;
                                        
                                        if (rect.bottom > availableHeight) {
                                            const scrollAmount = rect.bottom - availableHeight + 80;
                                            window.scrollBy(0, scrollAmount);
                                        }
                                    }
                                })();
                            """.trimIndent(), null)
                        }
                        insets
                    }

                    // Create and configure WebView
                    val wv = WebView(ctx).apply {
                        WebViewConfigurator.configureWebView(
                            webView = this,
                            context = ctx,
                            jsBridgeInterface = jsBridgeInterface,
                            contentReadyInterface = contentReadyInterface,
                            blobDownloadInterface = blobDownloadInterface,
                            jsBridgeController = jsBridgeController,
                            downloadHandler = downloadHandler,
                            onPageStarted = {
                                contentReady = false
                                progressComplete = false
                            },
                            onPageFinished = {
                                pageDone = true
                            },
                            onContentReady = {
                                contentReady = true
                            },
                            onProgressChanged = { progress ->
                                if (progress == 100) {
                                    progressComplete = true
                                }
                                downloadProgress = progress
                            }
                        )
                    }

                    addView(
                        wv,
                        FrameLayout.LayoutParams(
                            FrameLayout.LayoutParams.MATCH_PARENT,
                            FrameLayout.LayoutParams.MATCH_PARENT
                        )
                    )
                    webView = wv

                    // Add loader overlay
                    val loaderView = ComposeView(ctx).apply {
                        setContent {
                            LoadingOverlay(showLoader = showLoader)
                        }
                    }

                    addView(
                        loaderView,
                        FrameLayout.LayoutParams(
                            FrameLayout.LayoutParams.MATCH_PARENT,
                            FrameLayout.LayoutParams.MATCH_PARENT
                        )
                    )
                }
            },
            modifier = Modifier
                .fillMaxSize()
                .imePadding()
                .navigationBarsPadding()
        )

        // Load URL logic
        LaunchedEffect(urlToLoad) {
            urlToLoad?.let {
                pageDone = false
                contentReady = false
                progressComplete = false
                showLoader = true
                Log.d("WebViewContent2", "🌐 Loading: $it")
                webView?.loadUrl(it)
            }
        }

        // Hide loader when content is ready
        LaunchedEffect(contentReady, progressComplete) {
            if (contentReady && progressComplete) {
                kotlinx.coroutines.delay(200)
                showLoader = false
                Log.d("WebViewContent2", "✅ Hiding loader - Content is ready!")
            }
        }

        // Download Progress Dialog
        DownloadProgressDialog(
            showDownloadDialog = showDownloadDialog,
            downloadFileName = downloadFileName,
            downloadProgress = downloadProgress,
            onDismiss = { showDownloadDialog = false }
        )

        // Secret long-press zone
        SecretLongPressZone(
            onLongPress = { showFolderDialog = true }
        )

        // Folder selection dialog
        FolderSelectionDialog(
            showFolderDialog = showFolderDialog,
            onSelectFolder = { pickFolderLauncher.launch(null) },
            onDismiss = { showFolderDialog = false }
        )
    }
}

@HiltViewModel
class WebViewDependencyProvider @Inject constructor(
    val jsInterfaces: WebViewJSInterfaces,
    val downloadHandlerFactory: WebViewDownloadHandler.Factory
) : androidx.lifecycle.ViewModel()
