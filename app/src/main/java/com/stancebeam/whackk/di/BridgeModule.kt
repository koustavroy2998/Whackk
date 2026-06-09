package com.stancebeam.cc_lane.di

import com.stancebeam.cc_lane.data.JSBridge.JSBridgeController
import com.stancebeam.cc_lane.data.JSBridge.JSBridgeInterface
import dagger.Module
import dagger.Provides
import dagger.hilt.InstallIn
import dagger.hilt.components.SingletonComponent
import javax.inject.Singleton

@Module
@InstallIn(SingletonComponent::class)
object BridgeModule {

    @Provides
    @Singleton
    fun provideJSBridgeController(): JSBridgeController {
        return JSBridgeController()
    }

    @Provides
    @Singleton
    fun provideJSBridgeInterface(
        bridgeController: JSBridgeController
    ): JSBridgeInterface {
        return JSBridgeInterface(bridgeController)
    }
}
