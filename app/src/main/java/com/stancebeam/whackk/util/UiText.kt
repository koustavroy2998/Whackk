package com.stancebeam.cc_lane.util

import android.content.Context
import androidx.annotation.StringRes
import androidx.compose.runtime.Composable
import androidx.compose.ui.res.stringResource

sealed class UiText {

    data class DynamicString(val value: String) : UiText()

    data class StringResource(@StringRes val resId: Int) : UiText()

    @Composable
    fun asString(vararg args: Any): String {
        return when (this) {
            is DynamicString -> value
            is StringResource -> stringResource(resId, args)
        }
    }

    fun asString(context: Context, vararg args: Any): String {
        return when (this) {
            is DynamicString -> value
            is StringResource -> context.getString(resId, args)
        }
    }
}