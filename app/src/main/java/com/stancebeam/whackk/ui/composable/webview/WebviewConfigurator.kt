package com.stancebeam.cc_lane.ui.composable.webview

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.Bitmap
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.MotionEvent
import android.view.View
import android.webkit.*

object WebViewConfigurator {

    @SuppressLint("JavascriptInterface", "ClickableViewAccessibility")
    fun configureWebView(
        webView: WebView,
        context: Context,
        jsBridgeInterface: Any,
        contentReadyInterface: Any,
        blobDownloadInterface: Any,
        jsBridgeController: Any,
        downloadHandler: WebViewDownloadHandler,
        onPageStarted: () -> Unit,
        onPageFinished: () -> Unit,
        onContentReady: () -> Unit,
        onProgressChanged: (Int) -> Unit
    ) {
        webView.apply {
            // Basic settings
            settings.javaScriptEnabled = true
            settings.domStorageEnabled = true
            settings.cacheMode = WebSettings.LOAD_NO_CACHE
            settings.loadsImagesAutomatically = true
            settings.mixedContentMode = WebSettings.MIXED_CONTENT_ALWAYS_ALLOW
            settings.mediaPlaybackRequiresUserGesture = false

            // Enhanced keyboard support settings
            settings.setSupportZoom(false)
            settings.builtInZoomControls = false
            settings.displayZoomControls = false
            settings.useWideViewPort = true
            settings.loadWithOverviewMode = true
            settings.setRenderPriority(WebSettings.RenderPriority.HIGH)

            setLayerType(View.LAYER_TYPE_HARDWARE, null)

            // Focus handling
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
            addJavascriptInterface(blobDownloadInterface, "BlobDownloader")

            // Set bridge controller using reflection to avoid tight coupling
            try {
                val setWebViewMethod = jsBridgeController.javaClass.getMethod("setWebView", WebView::class.java)
                setWebViewMethod.invoke(jsBridgeController, this)
            } catch (e: Exception) {
                Log.w("WebViewConfigurator", "Could not set WebView on bridge controller: ${e.message}")
            }

            // Download listener
            setDownloadListener { url, userAgent, contentDisposition, mimeType, contentLength ->
                Log.d("WebViewContent2", "📥 Download requested: $url")
                downloadHandler.handleDownload(url, userAgent, contentDisposition, mimeType, contentLength, this)
            }

            // WebView client
            webViewClient = createWebViewClient(onPageStarted, onPageFinished, onContentReady)

            // Chrome client
            webChromeClient = createWebChromeClient(onProgressChanged, onContentReady)
        }
    }

    private fun createWebViewClient(
        onPageStarted: () -> Unit,
        onPageFinished: () -> Unit,
        onContentReady: () -> Unit
    ) = object : WebViewClient() {
        override fun onPageStarted(view: WebView?, url: String?, favicon: Bitmap?) {
            super.onPageStarted(view, url, favicon)
            Log.d("WebViewContent2", "🚀 Page started loading: $url")
            onPageStarted()
        }

        override fun onPageFinished(view: WebView?, url: String?) {
            super.onPageFinished(view, url)
            Log.d("WebViewContent2", "📄 Page HTML loaded: $url")
            onPageFinished()

            // Inject content detection JavaScript
            val contentDetectionJS = getContentDetectionJS()
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
            onContentReady() // Hide loader on error
        }
    }

    private fun createWebChromeClient(
        onProgressChanged: (Int) -> Unit,
        onContentReady: () -> Unit
    ) = object : WebChromeClient() {
        override fun onProgressChanged(view: WebView?, newProgress: Int) {
            super.onProgressChanged(view, newProgress)
            onProgressChanged(newProgress)

            if (newProgress == 100) {
                Log.d("WebViewContent2", "📊 Progress: 100% - Content loading complete")

                Handler(Looper.getMainLooper()).postDelayed({
                    onContentReady()
                }, 2000)
            } else {
                Log.d("WebViewContent2", "📊 Progress: $newProgress%")
            }
        }
    }

    private fun getContentDetectionJS() = """
        (function() {
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
                
                const hasVisibleContent = body.children.length > 0 && body.offsetHeight > 50;
                
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
}
