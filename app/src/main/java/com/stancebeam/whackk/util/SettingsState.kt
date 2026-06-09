package com.stancebeam.cc_lane.util

import kotlinx.coroutines.flow.Flow

data class SettingsState(
    val saveFiles: Flow<Boolean>,
    val setSaveFiles: (Boolean) -> Unit,
)