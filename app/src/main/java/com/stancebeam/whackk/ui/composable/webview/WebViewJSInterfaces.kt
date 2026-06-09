package com.stancebeam.cc_lane.ui.composable.webview

import android.content.Context
import android.os.Handler
import android.os.Looper
import android.util.Base64
import android.util.Log
import android.webkit.JavascriptInterface
import android.widget.Toast
import android.os.Environment
import java.io.File
import java.net.URLDecoder
import javax.inject.Inject

class WebViewJSInterfaces @Inject constructor(
    private val context: Context
) {
    
    fun createContentReadyInterface(
        onContentReady: () -> Unit,
        onReactAppReady: () -> Unit
    ) = object {
        @JavascriptInterface
        fun onContentReady() {
            Log.d("WebViewContent2", "✅ UI Content is ready and visible")
            Handler(Looper.getMainLooper()).post { onContentReady() }
        }

        @JavascriptInterface
        fun onReactAppReady() {
            Log.d("WebViewContent2", "⚛️ React App is fully loaded")
            Handler(Looper.getMainLooper()).post { onReactAppReady() }
        }
    }
    
    fun createBlobDownloadInterface(
        onDownloadComplete: () -> Unit,
        onDownloadError: (String) -> Unit
    ) = object {
        @JavascriptInterface
        fun downloadBlob(base64Data: String, fileName: String, mimeType: String) {
            try {
                val decodedBytes = Base64.decode(base64Data, Base64.DEFAULT)
                val downloadsDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS)
                val file = File(downloadsDir, fileName)
                file.writeBytes(decodedBytes)
                
                Log.d("WebViewContent2", "✅ Blob file saved: ${file.absolutePath}")
                
                Handler(Looper.getMainLooper()).post {
                    onDownloadComplete()
                    Toast.makeText(
                        context, 
                        "File downloaded: $fileName (${decodedBytes.size} bytes)", 
                        Toast.LENGTH_LONG
                    ).show()
                }
            } catch (e: Exception) {
                Log.e("WebViewContent2", "Blob download error: ${e.message}")
                Handler(Looper.getMainLooper()).post {
                    onDownloadError("Download failed: ${e.message}")
                }
            }
        }

        @JavascriptInterface
        fun downloadDataUrl(dataUrl: String, fileName: String) {
            try {
                Log.d("WebViewContent2", "📥 Processing data URL for: $fileName")
                
                val decodedData = URLDecoder.decode(dataUrl, "UTF-8")
                val base64Part = if (decodedData.contains(",")) {
                    decodedData.substringAfter(",")
                } else {
                    decodedData
                }
                
                val dataBytes = base64Part.toByteArray(Charsets.UTF_8)
                val downloadsDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS)
                val file = File(downloadsDir, fileName)
                file.writeBytes(dataBytes)
                
                Log.d("WebViewContent2", "✅ Data URL file saved: ${file.absolutePath}")
                
                Handler(Looper.getMainLooper()).post {
                    onDownloadComplete()
                    Toast.makeText(
                        context, 
                        "File downloaded: $fileName at (${file.absolutePath})", 
                        Toast.LENGTH_LONG
                    ).show()
                }
            } catch (e: Exception) {
                Log.e("WebViewContent2", "Data URL download error: ${e.message}")
                Handler(Looper.getMainLooper()).post {
                    onDownloadError("Download failed: ${e.message}")
                }
            }
        }
    }
}
