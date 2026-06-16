package com.stancebeam.cc_lane.ui.navigation

import androidx.compose.animation.AnimatedContentTransitionScope
import androidx.compose.animation.core.tween
import androidx.compose.runtime.Composable
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import com.stancebeam.cc_lane.ui.screen.splash.SplashScreen
import com.stancebeam.cc_lane.ui.screen.home.HomeScreen

import com.stancebeam.cc_lane.ui.screen.webview.WebViewScreen


@Composable
fun AppNavigation(startDestination: String) {
    val navController = rememberNavController()
    NavHost(
        navController = navController,
        startDestination = startDestination,
        enterTransition = {
            slideIntoContainer(AnimatedContentTransitionScope.SlideDirection.Left, tween(400))
        },
        exitTransition = {
            slideOutOfContainer(AnimatedContentTransitionScope.SlideDirection.Left, tween(400))
        },
        popEnterTransition = {
            slideIntoContainer(AnimatedContentTransitionScope.SlideDirection.Right, tween(400))
        },
        popExitTransition = {
            slideOutOfContainer(AnimatedContentTransitionScope.SlideDirection.Right, tween(400))
        },
    ) {

        composable(
            route = Destination.Splash.route,
        ) {
            SplashScreen(navController)
        }
        composable(
            route = Destination.HomeNav.route,
        ) {
            HomeScreen(navController)
        }
        composable(
            route = Destination.WebViewScreen.route,
        ) {
            WebViewScreen(navController)
        }


    }
}