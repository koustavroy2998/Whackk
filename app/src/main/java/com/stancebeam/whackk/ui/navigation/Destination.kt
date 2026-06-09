package com.stancebeam.cc_lane.ui.navigation

import androidx.navigation.NamedNavArgument

sealed class Destination(
    val route: String,
    val arguments: List<NamedNavArgument> = emptyList(),
) {


    data object Splash : Destination("splash")
    data object WebViewScreen : Destination("webViewScreen")

    data object AuthNav : Destination("authNav")
    data object HomeNav : Destination("homeNav")
//    data object Account : Destination("account")
//    data object Settings : Destination("settings")
//    data object StartSession : Destination("startSession")
//    data object LiveSession : Destination(
//        route = "liveSession/{sessionId}",
//        arguments = listOf(
//            navArgument("sessionId") {
//                type = NavType.StringType
//                nullable = false
//            },
//        ),
//    ) {
//
//        fun routeWithArgs(sessionId: String): String {
//            return route.replace("{sessionId}", sessionId)
//        }
//    }
}