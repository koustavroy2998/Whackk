package com.stancebeam.cc_lane.data

import androidx.room.Database
import androidx.room.RoomDatabase
import com.stancebeam.cc_lane.data.dao.BleSensorDao
import com.stancebeam.cc_lane.data.dao.PlayerDao
import com.stancebeam.cc_lane.data.dao.SessionDao
import com.stancebeam.cc_lane.data.dao.StrikerDao
import com.stancebeam.cc_lane.data.dao.SwingDao
import com.stancebeam.cc_lane.data.entity.BleSensor
import com.stancebeam.cc_lane.data.entity.Player
import com.stancebeam.cc_lane.data.entity.Session
import com.stancebeam.cc_lane.data.entity.SessionPlayerCrossRef
import com.stancebeam.cc_lane.data.entity.Striker
import com.stancebeam.cc_lane.data.entity.Swing

@Database(
    entities = [
        Player::class,
        Striker::class,
        Session::class,
        SessionPlayerCrossRef::class,
        Swing::class,
        BleSensor::class

    ],
    version = 4,
    exportSchema = false,
)
abstract class AppDatabase : RoomDatabase() {

    companion object {

        const val NAME = "StanceBeamCC.db"
    }

    abstract fun getPlayerDao(): PlayerDao

    abstract fun getStrikerDao(): StrikerDao

    abstract fun getSessionDao(): SessionDao

    abstract fun getSwingDao(): SwingDao
    abstract fun bleSensorDao(): BleSensorDao
}