package com.stancebeam.cc_lane.ui.navigation.auth

sealed class AuthScreen(val route: String) {

    data object Splash : AuthScreen("splash")
    data object SignIn : AuthScreen("signIn")
}