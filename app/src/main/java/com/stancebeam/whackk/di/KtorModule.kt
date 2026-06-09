package com.stancebeam.cc_lane.di

import com.stancebeam.cc_lane.data.network.WebSocketClient
import com.stancebeam.cc_lane.data.repository.PreferencesRepository
import dagger.Module
import dagger.Provides
import dagger.hilt.InstallIn
import dagger.hilt.components.SingletonComponent
import io.ktor.client.HttpClient
import io.ktor.client.engine.cio.CIO
import io.ktor.client.plugins.logging.Logging
import io.ktor.client.plugins.websocket.WebSockets
import javax.inject.Singleton

@Module
@InstallIn(SingletonComponent::class)
object KtorModule {

    @Provides
    @Singleton
    fun provideClient(): HttpClient {
        return HttpClient(CIO) {
            install(Logging)
            install(WebSockets)
        }
    }

    @Provides
    @Singleton
    fun provideWebSocketClient(client: HttpClient, preferences: PreferencesRepository): WebSocketClient {
        return WebSocketClient(client,preferences)
    }
}