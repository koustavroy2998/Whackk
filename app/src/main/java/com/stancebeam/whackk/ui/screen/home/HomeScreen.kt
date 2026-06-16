package com.stancebeam.cc_lane.ui.screen.home

import android.Manifest
import android.annotation.SuppressLint
import android.app.DownloadManager
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.graphics.Bitmap
import android.os.Environment
import android.os.Handler
import android.os.Looper
import android.util.Base64
import android.util.Log
import android.view.MotionEvent
import android.view.View
import android.webkit.JavascriptInterface
import android.webkit.WebChromeClient
import android.webkit.WebResourceError
import android.webkit.WebResourceRequest
import android.webkit.WebSettings
import android.webkit.WebView
import android.webkit.WebViewClient
import android.widget.FrameLayout
import android.widget.Toast
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.background
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.imePadding
import androidx.compose.foundation.layout.navigationBarsPadding
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.SnackbarHost
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.shadow
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.platform.ComposeView
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.viewinterop.AndroidView
import androidx.core.app.ActivityCompat
import androidx.core.app.NotificationCompat
import androidx.core.app.NotificationManagerCompat
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import androidx.hilt.navigation.compose.hiltViewModel
import androidx.navigation.NavHostController
import androidx.navigation.compose.rememberNavController
import com.airbnb.lottie.compose.LottieAnimation
import com.airbnb.lottie.compose.LottieCompositionSpec
import com.airbnb.lottie.compose.LottieConstants
import com.airbnb.lottie.compose.animateLottieCompositionAsState
import com.airbnb.lottie.compose.rememberLottieComposition

import com.stancebeam.whackk.ui.navigation.home.HomeNavItem

import kotlinx.coroutines.delay
import java.io.File
import java.net.URLDecoder
import androidx.core.net.toUri


private val homeNavItems = listOf(
    HomeNavItem.Connect,
    HomeNavItem.Play,
    HomeNavItem.Players,
)


@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun HomeScreen(
    rootNavController: NavHostController,
    navController: NavHostController = rememberNavController(),
    viewModel: HomeViewModel = hiltViewModel(),
) {
    val uiState by viewModel.uiState.collectAsState()
    val snackbarHostState = remember {
        SnackbarHostState()
    }
    Scaffold(
        snackbarHost = {
            SnackbarHost(snackbarHostState)
        },
    ) { innerPadding ->
        Box(
            modifier = Modifier
                //.padding(innerPadding)
                .fillMaxSize()
        ) {
            WebViewContent2(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(innerPadding), defaultUrl = uiState.url,viewModel
            )

        }
    }
}
@SuppressLint("ClickableViewAccessibility", "JavascriptInterface")
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun WebViewContent2(
    modifier: Modifier = Modifier,
    defaultUrl: String,
    viewModel: HomeViewModel
) {
    val context = LocalContext.current
    var webView by remember { mutableStateOf<WebView?>(null) }
    var showFolderDialog by remember { mutableStateOf(false) }

    val urlFromJson by viewModel.urlState.collectAsState()
    val folderUri by viewModel.folderUri.collectAsState()

    // URL to load only after JSON/folder check
    var urlToLoad by remember { mutableStateOf<String?>(null) }
    var pageDone by remember { mutableStateOf(false) }
    var contentReady by remember { mutableStateOf(false) }
    var showLoader by remember { mutableStateOf(true) }
    var progressComplete by remember { mutableStateOf(false) }

    // Download progress states
    var showDownloadDialog by remember { mutableStateOf(false) }
    var downloadProgress by remember { mutableStateOf(0) }
    var downloadFileName by remember { mutableStateOf("") }

    val jsBridgeController = remember { viewModel.getJSBridgeController() }
    val jsBridgeInterface = remember { viewModel.getJSBridgeInterface() }
    DisposableEffect(key1 = null) {
        onDispose {
            Log.d("Dispose->","WebViewContent2: Dispose")
            viewModel.release()
        }
    }

    LaunchedEffect(folderUri, urlFromJson) {
        urlToLoad = if (folderUri != null && !urlFromJson.isNullOrEmpty()) {
            urlFromJson
        } else defaultUrl
        pageDone = false
        contentReady = false
        progressComplete = false
        showLoader = true
    }

    val pickFolderLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.OpenDocumentTree()
    ) { uri ->
        uri?.let { viewModel.saveFolderUri(it) }
        showFolderDialog = false
        showLoader = false
        Log.d("WebViewContent2", "📂 Folder selected: $uri")
    }

    val micPermissionLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.RequestPermission()
    ) { granted ->
        viewModel.onMicPermissionResult(granted)
    }

    LaunchedEffect(Unit) {
        viewModel.requestMicPermission.collect {
            micPermissionLauncher.launch(Manifest.permission.RECORD_AUDIO)
        }
    }

    // Content Ready JavaScript Interface
    val contentReadyInterface = remember {
        object {
            @JavascriptInterface
            fun onContentReady() {
                Log.d("WebViewContent2", "✅ UI Content is ready and visible")
                contentReady = true
            }

            @JavascriptInterface
            fun onReactAppReady() {
                Log.d("WebViewContent2", "⚛️ React App is fully loaded")
                contentReady = true
            }
        }
    }

    // Enhanced Blob Download JavaScript Interface
    val blobDownloadInterface = remember {
        object {
            @JavascriptInterface
            fun downloadBlob(base64Data: String, fileName: String, mimeType: String) {
                try {
                    // Decode base64 to bytes
                    val decodedBytes = Base64.decode(base64Data, Base64.DEFAULT)

                    // Create file in Downloads directory
                    val downloadsDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS)
                    val file = File(downloadsDir, fileName)

                    // Write bytes to file
                    file.writeBytes(decodedBytes)

                    Log.d("WebViewContent2", "✅ Blob file saved: ${file.absolutePath}")

                    // Show notification
                    //   showDownloadNotification(fileName)

                    // Show toast
                    Handler(Looper.getMainLooper()).post {
                        showDownloadDialog = false
                        Toast.makeText(context, "File downloaded: $fileName (${decodedBytes.size} bytes)", Toast.LENGTH_LONG).show()
                    }

                } catch (e: Exception) {
                    Log.e("WebViewContent2", "Blob download error: ${e.message}")
                    Handler(Looper.getMainLooper()).post {
                        showDownloadDialog = false
                        Toast.makeText(context, "Download failed: ${e.message}", Toast.LENGTH_LONG).show()
                    }
                }
            }

            @JavascriptInterface
            fun downloadDataUrl(dataUrl: String, fileName: String) {
                try {
                    Log.d("WebViewContent2", "📥 Processing data URL for: $fileName")

                    // Decode the data URL
                    val decodedData = URLDecoder.decode(dataUrl, "UTF-8")

                    // Extract the base64 part after the comma
                    val base64Part = if (decodedData.contains(",")) {
                        decodedData.substringAfter(",")
                    } else {
                        decodedData
                    }

                    // Convert to bytes
                    val dataBytes = base64Part.toByteArray(Charsets.UTF_8)

                    // Create file in Downloads directory
                    val downloadsDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS)
                    val file = File(downloadsDir, fileName)

                    // Write bytes to file
                    file.writeBytes(dataBytes)

                    Log.d("WebViewContent2", "✅ Data URL file saved: ${file.absolutePath}")

                    // Show notification
                    // showDownloadNotification(fileName)

                    // Show toast
                    Handler(Looper.getMainLooper()).post {
                        showDownloadDialog = false
                        Toast.makeText(context, "File downloaded: $fileName at  ( ${file.absolutePath}\")", Toast.LENGTH_LONG).show()
                    }

                } catch (e: Exception) {
                    Log.e("WebViewContent2", "Data URL download error: ${e.message}")
                    Handler(Looper.getMainLooper()).post {
                        showDownloadDialog = false
                        Toast.makeText(context, "Download failed: ${e.message}", Toast.LENGTH_LONG).show()
                    }
                }
            }
        }
    }

    // Helper function for notifications
    fun showDownloadNotification(fileName: String) {
        try {
            val intent = Intent(DownloadManager.ACTION_VIEW_DOWNLOADS)
            val pendingIntent = PendingIntent.getActivity(
                context,
                0,
                intent,
                PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
            )

            val notificationBuilder = NotificationCompat.Builder(context, "downloads")
                .setSmallIcon(android.R.drawable.stat_sys_download_done)
                .setContentTitle("Download Complete")
                .setContentText(fileName)
                .setContentIntent(pendingIntent)
                .setAutoCancel(true)

            val notificationManager = NotificationManagerCompat.from(context)
            if (ActivityCompat.checkSelfPermission(
                    context,
                    Manifest.permission.POST_NOTIFICATIONS
                ) == PackageManager.PERMISSION_GRANTED
            ) {
                notificationManager.notify(System.currentTimeMillis().toInt(), notificationBuilder.build())
            }
        } catch (e: Exception) {
            Log.e("WebViewContent2", "Notification error: ${e.message}")
        }
    }

    // Enhanced download handler for HTTP/HTTPS/blob/data URLs
    fun handleDownload(url: String, userAgent: String, contentDisposition: String, mimeType: String, contentLength: Long) {
        try {
            Log.d("WebViewContent2", "📥 Download URL type: ${url.take(20)}...")

            // Extract filename from contentDisposition or use default
            val filename = if (contentDisposition.contains("filename=")) {
                contentDisposition.substringAfter("filename=")
                    .replace("\"", "")
                    .trim()
            } else {
                "download-${System.currentTimeMillis()}.json"
            }

            downloadFileName = filename
            showDownloadDialog = true
            downloadProgress = 0

            when {
                // Handle data URLs
                url.startsWith("data:") -> {
                    Log.d("WebViewContent2", "📥 Data URL detected, processing directly")

                    // Use JavaScript to process the data URL
                    val dataUrlJS = """
                        (function() {
                            const dataUrl = '$url';
                            const fileName = '$filename';
                            
                            console.log('Processing data URL for:', fileName);
                            BlobDownloader.downloadDataUrl(dataUrl, fileName);
                        })();
                    """.trimIndent()

                    webView?.evaluateJavascript(dataUrlJS, null)

                    // Hide dialog after processing
                    Handler(Looper.getMainLooper()).postDelayed({
                        if (showDownloadDialog) {
                            showDownloadDialog = false
                        }
                    }, 3000)
                }

                // Handle blob URLs
                url.startsWith("blob:") -> {
                    Log.d("WebViewContent2", "📥 Blob URL detected, using JavaScript conversion")

                    // Use JavaScript to convert blob to base64 and download
                    val blobToBase64JS = """
                        (function() {
                            const blobUrl = '$url';
                            
                            console.log('Processing blob URL for:', '$filename');
                            
                            fetch(blobUrl)
                                .then(response => {
                                    if (!response.ok) {
                                        throw new Error('Failed to fetch blob: ' + response.statusText);
                                    }
                                    return response.blob();
                                })
                                .then(blob => {
                                    console.log('Blob size:', blob.size, 'bytes');
                                    const reader = new FileReader();
                                    reader.onloadend = function() {
                                        const base64data = reader.result.split(',')[1]; // Remove data:mime;base64, prefix
                                        BlobDownloader.downloadBlob(base64data, '$filename', '$mimeType');
                                    };
                                    reader.onerror = function(error) {
                                        console.error('FileReader error:', error);
                                    };
                                    reader.readAsDataURL(blob);
                                })
                                .catch(error => {
                                    console.error('Blob conversion error:', error);
                                });
                        })();
                    """.trimIndent()

                    webView?.evaluateJavascript(blobToBase64JS, null)

                    // Hide dialog after processing
                    Handler(Looper.getMainLooper()).postDelayed({
                        if (showDownloadDialog) {
                            showDownloadDialog = false
                        }
                    }, 5000)
                }

                // Handle regular HTTP/HTTPS downloads
                url.startsWith("http://") || url.startsWith("https://") -> {
                    Log.d("WebViewContent2", "📥 HTTP/HTTPS URL detected, using DownloadManager")

                    val downloadManager = context.getSystemService(Context.DOWNLOAD_SERVICE) as DownloadManager
                    val request = DownloadManager.Request(url.toUri()).apply {
                        setTitle(filename)
                        setDescription("Downloading file...")
                        setNotificationVisibility(DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED)
                        setDestinationInExternalPublicDir(Environment.DIRECTORY_DOWNLOADS, filename)
                        setMimeType(mimeType)
                        addRequestHeader("User-Agent", userAgent)
                    }

                    val downloadId = downloadManager.enqueue(request)
                    Log.d("WebViewContent2", "📥 HTTP Download started: $downloadFileName")

                    // Track download progress for HTTP downloads
                    val progressTracker = object : Runnable {
                        override fun run() {
                            val query = DownloadManager.Query().setFilterById(downloadId)
                            val cursor = downloadManager.query(query)

                            if (cursor.moveToFirst()) {
                                val bytesDownloaded = cursor.getInt(cursor.getColumnIndexOrThrow(DownloadManager.COLUMN_BYTES_DOWNLOADED_SO_FAR))
                                val bytesTotal = cursor.getInt(cursor.getColumnIndexOrThrow(DownloadManager.COLUMN_TOTAL_SIZE_BYTES))
                                val status = cursor.getInt(cursor.getColumnIndexOrThrow(DownloadManager.COLUMN_STATUS))

                                if (bytesTotal > 0) {
                                    downloadProgress = (bytesDownloaded * 100 / bytesTotal)
                                }

                                when (status) {
                                    DownloadManager.STATUS_SUCCESSFUL -> {
                                        showDownloadDialog = false
                                        Log.d("WebViewContent2", "✅ HTTP Download completed: $downloadFileName")
                                    }
                                    DownloadManager.STATUS_FAILED -> {
                                        showDownloadDialog = false
                                        Log.e("WebViewContent2", "❌ HTTP Download failed: $downloadFileName")
                                    }
                                    else -> {
                                        Handler(Looper.getMainLooper()).postDelayed(this, 500)
                                    }
                                }
                            }
                            cursor.close()
                        }
                    }

                    Handler(Looper.getMainLooper()).post(progressTracker)
                }

                else -> {
                    Log.e("WebViewContent2", "❌ Unsupported URL scheme: $url")
                    showDownloadDialog = false
                    Toast.makeText(context, "Unsupported download URL", Toast.LENGTH_LONG).show()
                }
            }

        } catch (e: Exception) {
            Log.e("WebViewContent2", "Download error: ${e.message}")
            showDownloadDialog = false
            Toast.makeText(context, "Download failed: ${e.message}", Toast.LENGTH_LONG).show()
        }
    }

    Box(modifier = modifier.fillMaxSize()) {
        AndroidView(
            factory = { ctx ->
                FrameLayout(ctx).apply {
                    layoutParams = FrameLayout.LayoutParams(
                        FrameLayout.LayoutParams.MATCH_PARENT,
                        FrameLayout.LayoutParams.MATCH_PARENT
                    )

                    // Handle window insets for keyboard with immediate response
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

                        } else {
                            Log.d("WebViewContent2", "⌨️ Keyboard hidden")
                        }

                        insets
                    }

                    // WebView with enhanced performance settings
                    val wv = WebView(ctx).apply {
                        settings.javaScriptEnabled = true
                        settings.domStorageEnabled = true
                        settings.cacheMode = WebSettings.LOAD_DEFAULT
                        settings.loadsImagesAutomatically = true
                        settings.mixedContentMode = WebSettings.MIXED_CONTENT_ALWAYS_ALLOW
                        settings.mediaPlaybackRequiresUserGesture = false

                        settings.setSupportZoom(false)
                        settings.builtInZoomControls = false
                        settings.displayZoomControls = false
                        settings.useWideViewPort = true
                        settings.loadWithOverviewMode = true
                        settings.setRenderPriority(WebSettings.RenderPriority.HIGH)

                        setLayerType(View.LAYER_TYPE_HARDWARE, null)

                        requestFocus(View.FOCUS_DOWN)
                        setOnTouchListener { v, event ->
                            when (event.action) {
                                MotionEvent.ACTION_DOWN, MotionEvent.ACTION_UP -> {
                                    if (!v.hasFocus()) {
                                        v.requestFocus()
                                    }
                                }
                            }
                            false
                        }

                        // Add JavaScript Interfaces
                        addJavascriptInterface(jsBridgeInterface, "AndroidBridge")
                        addJavascriptInterface(contentReadyInterface, "ContentReady")
                        addJavascriptInterface(blobDownloadInterface, "BlobDownloader") // Add enhanced blob downloader

                        jsBridgeController.setWebView(this)

                        // Add Download Listener for all download attempts
                        setDownloadListener { url, userAgent, contentDisposition, mimeType, contentLength ->
                            Log.d("WebViewContent2", "📥 Download requested: $url")
                            handleDownload(url, userAgent, contentDisposition, mimeType, contentLength)
                        }

                        // Rest of your existing WebViewClient and WebChromeClient code...
                        webViewClient = object : WebViewClient() {
                            override fun onPageStarted(view: WebView?, url: String?, favicon: Bitmap?) {
                                super.onPageStarted(view, url, favicon)
                                Log.d("WebViewContent2", "🚀 Page started loading: $url")
                                contentReady = false
                                progressComplete = false
                            }

                            override fun onPageFinished(view: WebView?, url: String?) {
                                super.onPageFinished(view, url)
                                Log.d("WebViewContent2", "📄 Page HTML loaded: $url")
                                pageDone = true

                                // Your existing content detection JavaScript
                                val contentDetectionJS = """
                                    (function() {
                                        // Fast keyboard and focus handling with instant scrolling
                                        function setupKeyboardHandling() {
                                            document.addEventListener('focusin', function(e) {
                                                if (e.target.tagName === 'INPUT' || e.target.tagName === 'TEXTAREA') {
                                                    console.log('Input focused, immediate scroll');
                                                    e.target.scrollIntoView({
                                                        behavior: 'instant',
                                                        block: 'center',
                                                        inline: 'nearest'
                                                    });
                                                }
                                            });
                                            
                                            let initialViewportHeight = window.innerHeight;
                                            
                                            window.addEventListener('resize', function() {
                                                const currentHeight = window.innerHeight;
                                                const heightDifference = initialViewportHeight - currentHeight;
                                                
                                                if (heightDifference > 150) {
                                                    console.log('Keyboard detected, instant adjustment');
                                                    const activeElement = document.activeElement;
                                                    if (activeElement && (activeElement.tagName === 'INPUT' || activeElement.tagName === 'TEXTAREA')) {
                                                        activeElement.scrollIntoView({
                                                            behavior: 'instant',
                                                            block: 'center',
                                                            inline: 'nearest'
                                                        });
                                                    }
                                                }
                                            });
                                        }
                                        
                                        function checkContentReady() {
                                            if (document.readyState !== 'complete') {
                                                setTimeout(checkContentReady, 50);
                                                return;
                                            }
                                            
                                            const body = document.body;
                                            if (!body || body.offsetHeight <= 0) {
                                                setTimeout(checkContentReady, 50);
                                                return;
                                            }
                                            
                                            setupKeyboardHandling();
                                            
                                            if (window.React || document.querySelector('[data-reactroot]') || document.querySelector('#root')) {
                                                const rootElement = document.querySelector('#root') || document.querySelector('[data-reactroot]');
                                                if (rootElement && rootElement.children.length > 0) {
                                                    console.log('React app detected and ready');
                                                    ContentReady.onReactAppReady();
                                                    return;
                                                }
                                            }
                                            
                                            const hasVisibleContent = body.children.length > 0 && 
                                                                     body.offsetHeight > 50;
                                            
                                            if (hasVisibleContent) {
                                                console.log('Content is ready and visible');
                                                ContentReady.onContentReady();
                                            } else {
                                                setTimeout(checkContentReady, 100);
                                            }
                                        }
                                        
                                        checkContentReady();
                                        
                                        window.addEventListener('load', function() {
                                            setTimeout(checkContentReady, 100);
                                        });
                                        
                                        if (window.React) {
                                            setTimeout(checkContentReady, 200);
                                        }
                                    })();
                                """.trimIndent()

                                view?.evaluateJavascript(contentDetectionJS, null)
                            }

                            override fun onPageCommitVisible(view: WebView?, url: String?) {
                                super.onPageCommitVisible(view, url)
                                Log.d("WebViewContent2", "👁️ Page is now visible: $url")
                            }

                            override fun onReceivedError(
                                view: WebView?,
                                request: WebResourceRequest?,
                                error: WebResourceError?
                            ) {
                                Log.e("WebViewContent2", "❌ WebView error: ${error?.description}")
                                pageDone = true
                                contentReady = true
                            }
                        }

                        webChromeClient = object : WebChromeClient() {
                            override fun onProgressChanged(view: WebView?, newProgress: Int) {
                                super.onProgressChanged(view, newProgress)

                                if (newProgress == 100) {
                                    progressComplete = true
                                    Log.d("WebViewContent2", "📊 Progress: 100% - Content loading complete")

                                    Handler(Looper.getMainLooper()).postDelayed({
                                        if (!contentReady) {
                                            Log.d("WebViewContent2", "⏰ Fallback: Hiding loader after timeout")
                                            contentReady = true
                                        }
                                    }, 2000)
                                } else {
                                    Log.d("WebViewContent2", "📊 Progress: $newProgress%")
                                }
                            }
                        }
                    }

                    addView(
                        wv,
                        FrameLayout.LayoutParams(
                            FrameLayout.LayoutParams.MATCH_PARENT,
                            FrameLayout.LayoutParams.MATCH_PARENT
                        )
                    )
                    webView = wv

                    // Your existing loader overlay code...
                    val loaderView = ComposeView(ctx).apply {
                        setContent {
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
                                        LottieCompositionSpec.Asset("ball.lottie")
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

        // Your existing LaunchedEffect and dialog code...
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

        LaunchedEffect(contentReady, progressComplete) {
            if (contentReady && progressComplete) {
                delay(200)
                showLoader = false
                Log.d("WebViewContent2", "✅ Hiding loader - Content is ready!")
            }
        }



        // Secret long-press zone
        Box(
            modifier = Modifier
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
                                showFolderDialog = true
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

        // Folder selection dialog
        if (showFolderDialog) {
            AlertDialog(
                onDismissRequest = { showFolderDialog = false },
                title = { Text("Select LaneConfig Folder") },
                text = { Text("Please select the LaneConfig folder to load the URL.") },
                confirmButton = {
                    Button(
                        onClick = { pickFolderLauncher.launch(null) },
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
                        onClick = { showFolderDialog = false },
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
}

