package com.stancebeam.cc_lane.ui.navigation.home

sealed class HomeNavTab(val route: String) {

    data object Play : HomeNavTab("play")
    data object Connect : HomeNavTab("connect")
    data object Players : HomeNavTab("players")
}