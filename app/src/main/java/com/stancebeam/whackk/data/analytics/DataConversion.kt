package com.stancebeam.cc_lane.data.analytics

object DataConversion {

    private const val ACC_SENSITIVITY = 2048
    private const val GYRO_SENSITIVITY = 16.4

    private fun convertFAToRealData(hexValue: String): List<Double> {
        val ts = hexValue.substring(2, 8)
            .toInt(16)
            .toDouble() * 2
        val strAccGyroSF = listOf(
            hexValue.substring(8, 12),
            hexValue.substring(12, 16),
            hexValue.substring(16, 18) + "00",
            hexValue.substring(18, 22),
            hexValue.substring(22, 26),
            hexValue.substring(26, 30),
            hexValue.substring(30, 34),
            hexValue.substring(34, 38),
            hexValue.substring(38)
        )
        val realData = mutableListOf(ts)
        strAccGyroSF.forEachIndexed { index, item ->
            val strNormLint = if (index == strAccGyroSF.lastIndex) "FF" else "FFFF"
            val intFirstChar = try {
                item.first()
                    .digitToInt() < 8
            } catch (e: Exception) {
                false
            }
            var value = if (intFirstChar) {
                item.toInt(16)
            } else {
                val normLint = strNormLint.toInt(16) - (item.toInt(16) - 1)
                normLint * -1
            }.toDouble()
            if (index < 3) {
                value /= ACC_SENSITIVITY
            } else if (index < 6) {
                value /= GYRO_SENSITIVITY
            }
            realData.add(value)
        }
        return realData
    }

    private fun convertFBToRealData(hexValue: String): List<Double> {
        val sequence = hexValue.substring(32)
            .toInt(16)
            .toDouble()
        val realData = mutableListOf(sequence)
        val strMagRawAcc = listOf(
            hexValue.substring(2, 6),
            hexValue.substring(6, 10),
            hexValue.substring(10, 14),
            hexValue.substring(14, 18),
            hexValue.substring(18, 22),
            hexValue.substring(22, 26),
        )
        strMagRawAcc.forEachIndexed { index, item ->
            val intFirstChar = try {
                item.first()
                    .digitToInt() < 8
            } catch (e: Exception) {
                false
            }
            var value = if (intFirstChar) {
                item.toInt(16)
            } else {
                val normLint = "FFFF".toInt(16) - (item.toInt(16) - 1)
                normLint * -1
            }.toDouble()
            if (index >= 3) {
                value /= ACC_SENSITIVITY
            }
            realData.add(value)
        }
        return realData
    }

    fun prepareDataForAE(faPackets: List<String>, fbPackets: List<String>): List<DoubleArray> {
        val ts = mutableListOf<Double>()
        val gyroX = mutableListOf<Double>()
        val gyroY = mutableListOf<Double>()
        val gyroZ = mutableListOf<Double>()
        val yaw = mutableListOf<Double>()
        val pitch = mutableListOf<Double>()
        val roll = mutableListOf<Double>()
        faPackets.forEach {
            val realData = convertFAToRealData(it)
            ts.add(realData[0])
            gyroX.add(realData[4])
            gyroY.add(realData[5])
            gyroZ.add(realData[6])
            yaw.add(realData[7])
            pitch.add(realData[8])
            roll.add(realData[9])
        }
        val magX = mutableListOf<Double>()
        val magY = mutableListOf<Double>()
        val magZ = mutableListOf<Double>()
        val accX = mutableListOf<Double>()
        val accY = mutableListOf<Double>()
        val accZ = mutableListOf<Double>()
        fbPackets.forEach {
            val realData = convertFBToRealData(it)
            magX.add(realData[1])
            magY.add(realData[2])
            magZ.add(realData[3])
            accX.add(realData[4])
            accY.add(realData[5])
            accZ.add(realData[6])
        }
        val acc = (ts + accX + accY + accZ).toDoubleArray()
        val gyro = (ts + gyroX + gyroY + gyroZ).toDoubleArray()
        val angles = (yaw + pitch + roll).toDoubleArray()
        val mag = (magX + magY + magZ).toDoubleArray()
        val magMaxMin = doubleArrayOf(
            magX.maxOrNull() ?: 0.0,
            magY.maxOrNull() ?: 0.0,
            magZ.maxOrNull() ?: 0.0,
            magX.minOrNull() ?: 0.0,
            magY.minOrNull() ?: 0.0,
            magZ.minOrNull() ?: 0.0,
        )
        return listOf(acc, gyro, angles, mag, magMaxMin)
    }
}