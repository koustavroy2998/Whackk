package com.stancebeam.whackk

import android.app.Application
import com.parse.Parse
import dagger.hilt.android.HiltAndroidApp

@HiltAndroidApp
class MainApplication : Application() {

    companion object {

        private const val APP_ID = "AA4071F5A30CA6525E2AED19ABA3F77AF72F90592412DBB707D55BCE7A86F3FDDBAB591E34D6014E6B8E1CC2F2ACD57E9D3731CD97F509EC65CDF7AC0A779897"
    }

    override fun onCreate() {
        super.onCreate()
        val configuration = Parse.Configuration.Builder(this)
            .applicationId(APP_ID)
            .server("https://strikerapiprod.azurewebsites.net/")
            .build()
        Parse.initialize(configuration)
    }
}