package com.stancebeam.cc_lane.ui.composable.webview

import android.app.DownloadManager
import android.content.Context
import android.net.Uri
import android.os.Environment
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.webkit.WebView
import javax.inject.Inject

class WebViewDownloadHandler @Inject constructor(
    private val context: Context
) {

    private var onDownloadStart: ((String) -> Unit)? = null
    private var onDownloadComplete: (() -> Unit)? = null
    private var onDownloadError: ((String) -> Unit)? = null

    fun setCallbacks(
        onDownloadStart: (String) -> Unit,
        onDownloadComplete: () -> Unit,
        onDownloadError: (String) -> Unit
    ) {
        this.onDownloadStart = onDownloadStart
        this.onDownloadComplete = onDownloadComplete
        this.onDownloadError = onDownloadError
    }

    fun handleDownload(
        url: String,
        userAgent: String,
        contentDisposition: String,
        mimeType: String,
        contentLength: Long,
        webView: WebView?
    ) {
        try {
            Log.d("WebViewDownload", "📥 Download URL type: ${url.take(20)}...")

            val filename = extractFileName(contentDisposition, url)
            onDownloadStart?.invoke(filename)

            when {
                url.startsWith("data:") -> handleDataUrl(url, filename, webView)
                url.startsWith("blob:") -> handleBlobUrl(url, filename, mimeType, webView)
                url.startsWith("http://") || url.startsWith("https://") -> {
                    handleHttpDownload(url, userAgent, filename, mimeType)
                }
                else -> {
                    Log.e("WebViewDownload", "❌ Unsupported URL scheme: $url")
                    onDownloadError?.invoke("Unsupported download URL")
                }
            }
        } catch (e: Exception) {
            Log.e("WebViewDownload", "Download error: ${e.message}")
            onDownloadError?.invoke("Download failed: ${e.message}")
        }
    }

    private fun extractFileName(contentDisposition: String, url: String): String {
        return if (contentDisposition.contains("filename=")) {
            contentDisposition.substringAfter("filename=")
                .replace("\"", "")
                .trim()
        } else {
            "download-${System.currentTimeMillis()}.json"
        }
    }

    private fun handleDataUrl(url: String, filename: String, webView: WebView?) {
        Log.d("WebViewDownload", "📥 Data URL detected, processing directly")

        val dataUrlJS = """
            (function() {
                const dataUrl = '$url';
                const fileName = '$filename';
                
                console.log('Processing data URL for:', fileName);
                BlobDownloader.downloadDataUrl(dataUrl, fileName);
            })();
        """.trimIndent()

        webView?.evaluateJavascript(dataUrlJS, null)

        Handler(Looper.getMainLooper()).postDelayed({
            onDownloadComplete?.invoke()
        }, 3000)
    }

    private fun handleBlobUrl(url: String, filename: String, mimeType: String, webView: WebView?) {
        Log.d("WebViewDownload", "📥 Blob URL detected, using JavaScript conversion")

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
                            const base64data = reader.result.split(',')[1];
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

        Handler(Looper.getMainLooper()).postDelayed({
            onDownloadComplete?.invoke()
        }, 5000)
    }

    private fun handleHttpDownload(url: String, userAgent: String, filename: String, mimeType: String) {
        Log.d("WebViewDownload", "📥 HTTP/HTTPS URL detected, using DownloadManager")

        val downloadManager = context.getSystemService(Context.DOWNLOAD_SERVICE) as DownloadManager
        val request = DownloadManager.Request(Uri.parse(url)).apply {
            setTitle(filename)
            setDescription("Downloading file...")
            setNotificationVisibility(DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED)
            setDestinationInExternalPublicDir(Environment.DIRECTORY_DOWNLOADS, filename)
            setMimeType(mimeType)
            addRequestHeader("User-Agent", userAgent)
        }

        downloadManager.enqueue(request)
        onDownloadComplete?.invoke()
    }

    class Factory @Inject constructor(
        private val context: Context
    ) {
        fun create(): WebViewDownloadHandler {
            return WebViewDownloadHandler(context)
        }
    }
}
