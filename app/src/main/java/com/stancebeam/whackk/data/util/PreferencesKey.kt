package com.stancebeam.cc_lane.data.util

import androidx.datastore.preferences.core.booleanPreferencesKey
import androidx.datastore.preferences.core.stringPreferencesKey

object PreferencesKey {

    val CURRENT_USER = stringPreferencesKey("currentUser")
    val DARK_THEME = booleanPreferencesKey("darkTheme")
    val SAVE_FILES = booleanPreferencesKey("saveFiles")
}