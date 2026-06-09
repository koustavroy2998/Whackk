package com.stancebeam.cc_lane.di

import com.stancebeam.cc_lane.data.repository.BleSensorRepository
import com.stancebeam.cc_lane.data.repository.BleSensorRepositoryImpl
import dagger.Binds
import dagger.Module
import dagger.hilt.InstallIn
import dagger.hilt.components.SingletonComponent

@Module
@InstallIn(SingletonComponent::class)
abstract class RepositoryModule {
    @Binds
    abstract fun bindBleSensorRepository(
        bleSensorRepositoryImpl: BleSensorRepositoryImpl
    ): BleSensorRepository
}