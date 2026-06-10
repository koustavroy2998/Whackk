package com.stancebeam.cc_lane.ui.screen.webview

data class WebViewState(
    var isLoading: Boolean = false,
    var showConnectionSheet: Boolean = false,
    val url: String = "http://192.168.0.192:5176"// main hosted url
    //val url: String = "http://192.168.0.91:5176/?facilityCode=BLR01"// dev hosted url
  // val url: String = "http://172.16.10.2:8080/?facilityCode=HOU01"// US URL;[
  // val url: String = "https://www.stancebeam.com/"// US URL;[
   // val url: String = "https://houston.century.cricket",///ws://192.168.0.111:9090/ws
)