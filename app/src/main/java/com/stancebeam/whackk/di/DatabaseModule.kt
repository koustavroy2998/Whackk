package com.stancebeam.cc_lane.di

import android.content.Context
import androidx.room.Room
import com.stancebeam.cc_lane.data.AppDatabase
import com.stancebeam.cc_lane.data.dao.BleSensorDao
import com.stancebeam.cc_lane.data.dao.PlayerDao
import com.stancebeam.cc_lane.data.dao.SessionDao
import com.stancebeam.cc_lane.data.dao.StrikerDao
import com.stancebeam.cc_lane.data.dao.SwingDao
import dagger.Module
import dagger.Provides
import dagger.hilt.InstallIn
import dagger.hilt.android.qualifiers.ApplicationContext
import dagger.hilt.components.SingletonComponent
import javax.inject.Singleton

@Module
@InstallIn(SingletonComponent::class)
object DatabaseModule {

    @Provides
    @Singleton
    fun provideDatabase(@ApplicationContext context: Context): AppDatabase {
        return Room.databaseBuilder(context, AppDatabase::class.java, AppDatabase.NAME)
            .fallbackToDestructiveMigration()
            .build()
    }

    @Provides
    @Singleton
    fun providePlayerDao(database: AppDatabase): PlayerDao {
        return database.getPlayerDao()
    }

    @Provides
    @Singleton
    fun provideStrikerDao(database: AppDatabase): StrikerDao {
        return database.getStrikerDao()
    }

    @Provides
    @Singleton
    fun provideSessionDao(database: AppDatabase): SessionDao {
        return database.getSessionDao()
    }

    @Provides
    @Singleton
    fun provideSwingDao(database: AppDatabase): SwingDao {
        return database.getSwingDao()
    }

    @Provides
    fun provideBleSensorDao(database: AppDatabase): BleSensorDao {
        return database.bleSensorDao()
    }
}

