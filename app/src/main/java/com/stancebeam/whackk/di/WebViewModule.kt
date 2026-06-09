package com.stancebeam.cc_lane.di

import android.content.Context
import com.stancebeam.cc_lane.ui.composable.webview.WebViewDownloadHandler
import com.stancebeam.cc_lane.ui.composable.webview.WebViewJSInterfaces

import dagger.Module
import dagger.Provides
import dagger.hilt.InstallIn
import dagger.hilt.android.components.ViewModelComponent
import dagger.hilt.android.qualifiers.ApplicationContext
import dagger.hilt.android.scopes.ViewModelScoped

@Module
@InstallIn(ViewModelComponent::class)
object WebViewModule {

    @Provides
    @ViewModelScoped
    fun provideWebViewJSInterfaces(
        @ApplicationContext context: Context
    ): WebViewJSInterfaces {
        return WebViewJSInterfaces(context)
    }

    @Provides
    @ViewModelScoped
    fun provideWebViewDownloadHandler(
        @ApplicationContext context: Context
    ): WebViewDownloadHandler.Factory {
        return WebViewDownloadHandler.Factory(context)
    }
}
