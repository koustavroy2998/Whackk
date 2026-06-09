package com.stancebeam.cc_lane.data.repository

import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.core.edit
import com.stancebeam.cc_lane.data.network.model.User
import com.stancebeam.cc_lane.data.util.PreferencesKey
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.map
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json

class PreferencesRepository(
    private val dataStore: DataStore<Preferences>
) {

    val currentUserFlow: Flow<User?> = dataStore.data.map {
        it[PreferencesKey.CURRENT_USER]?.let { currentUser ->
            Json.decodeFromString<User>(currentUser)
        }
    }
    val darkThemeFlow: Flow<Boolean?> = dataStore.data.map { preferences ->
        preferences[PreferencesKey.DARK_THEME]
    }
    val saveFilesFlow: Flow<Boolean> = dataStore.data.map { preferences ->
        preferences[PreferencesKey.SAVE_FILES] == true
    }

    suspend fun setCurrentUser(value: User?) {
        dataStore.edit { preferences ->
            if (value != null) {
                preferences[PreferencesKey.CURRENT_USER] = Json.encodeToString(value)
            } else {
                preferences.remove(PreferencesKey.CURRENT_USER)
            }
        }
    }

    suspend fun setDarkTheme(value: Boolean?) {
        dataStore.edit { preferences ->
            if (value != null) {
                preferences[PreferencesKey.DARK_THEME] = value
            } else {
                preferences.remove(PreferencesKey.DARK_THEME)
            }
        }
    }

    suspend fun setSaveFiles(value: Boolean) {
        dataStore.edit { preferences ->
            preferences[PreferencesKey.SAVE_FILES] = value
        }
    }
}