package com.stancebeam.cc_lane.di

import android.content.Context
import com.stancebeam.cc_lane.data.bluetooth.BluetoothController
import com.stancebeam.cc_lane.data.dataStore
import com.stancebeam.cc_lane.data.network.repository.AuthRepository
import com.stancebeam.cc_lane.data.repository.PreferencesRepository
import com.stancebeam.cc_lane.data.util.PermissionController
import dagger.Module
import dagger.Provides
import dagger.hilt.InstallIn
import dagger.hilt.android.qualifiers.ApplicationContext
import dagger.hilt.components.SingletonComponent
import javax.inject.Singleton

@Module
@InstallIn(SingletonComponent::class)
object AppModule {

    @Provides
    @Singleton
    fun provideBluetoothController(@ApplicationContext context: Context): BluetoothController {
        return BluetoothController(context)
    }

    @Provides
    @Singleton
    fun providePermissionController(@ApplicationContext context: Context): PermissionController {
        return PermissionController(context)
    }

    @Provides
    @Singleton
    fun provideAuthRepository(): AuthRepository {
        return AuthRepository()
    }

    @Provides
    @Singleton
    fun providePreferencesRepository(@ApplicationContext context: Context): PreferencesRepository {
        return PreferencesRepository(context.dataStore)
    }
}