package com.stancebeam.whackk.ui.navigation.home

import androidx.annotation.DrawableRes
import com.stancebeam.whackk.R


sealed class HomeNavItem(
    val route: String,
    val label: String,
    @DrawableRes val icon: Int,
    @DrawableRes val selectedIcon: Int,
) {

    data object Play : HomeNavItem(
        route = "play",
        label = "Play",
        icon = R.drawable.ic_play_arrow_outlined,
        selectedIcon = R.drawable.ic_play_arrow,
    )

    data object Connect : HomeNavItem(
        route = "connect",
        label = "Connect",
        icon = R.drawable.ic_link,
        selectedIcon = R.drawable.ic_link,
    )

    data object Players : HomeNavItem(
        route = "players",
        label = "Players",
        icon = R.drawable.ic_group_outlined,
        selectedIcon = R.drawable.ic_group,
    )
}