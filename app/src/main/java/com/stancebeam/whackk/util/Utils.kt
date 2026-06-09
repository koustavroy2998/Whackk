package com.stancebeam.cc_lane.util

import android.os.Environment
import com.stancebeam.cc_lane.data.entity.Session
import java.io.File
import java.time.Instant
import java.time.ZoneId
import java.time.format.DateTimeFormatter
import java.util.Date
import java.util.UUID

fun randomId(): String {
    return UUID.randomUUID()
        .toString()
}

fun timestamp(): Long {
    return Date().time
}

fun formatTimestamp(millis: Long, pattern: String = "dd-MM-yyyy hh-mm-ss"): String {
    return Instant.ofEpochMilli(millis)
        .atZone(ZoneId.systemDefault())
        .format(
            DateTimeFormatter.ofPattern(pattern)
        )
}

fun getSessionDir(session: Session): File {
    val downloadsDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS)
    val appDir = File(downloadsDir, "StanceBeamCC").apply {
        if (exists()) return@apply
        mkdirs()
    }
    val date = formatTimestamp(session.startedAt)
    val sessionDir = File(appDir, date).apply {
        if (exists()) return@apply
        mkdirs()
    }
    return sessionDir
}