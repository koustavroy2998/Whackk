package com.stancebeam.cc_lane.ui.navigation.auth

import androidx.navigation.NavGraphBuilder
import androidx.navigation.NavHostController
import androidx.navigation.compose.composable
import androidx.navigation.navigation
import com.stancebeam.cc_lane.ui.navigation.Destination

import com.stancebeam.cc_lane.ui.screen.splash.SplashScreen

fun NavGraphBuilder.authNavGraph(
    navController: NavHostController,
    route: String = Destination.AuthNav.route,
) {
    navigation(
        startDestination = AuthScreen.Splash.route,
        route = route,
    ) {
        composable(
            route = AuthScreen.Splash.route,
        ) {
            SplashScreen(navController)
        }
//        composable(
//            route = AuthScreen.SignIn.route,
//        ) {
//            SignInScreen(navController)
//        }
    }
}