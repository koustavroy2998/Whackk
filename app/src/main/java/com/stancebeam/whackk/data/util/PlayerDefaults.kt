package com.stancebeam.cc_lane.data.util

sealed class PlayerDefaults {

    data object Gender : PlayerDefaults() {

        const val MALE = "male"
        const val FEMALE = "female"
    }

    companion object {

        const val HEIGHT = 170
        const val WEIGHT = 70
    }

    data object BattingHand : PlayerDefaults() {

        const val LEFT = 0
        const val RIGHT = 1
    }

    data object GripPosition : PlayerDefaults() {

        const val LOW = 0
        const val MEDIUM = 1
        const val HIGH = 2
    }

    data object BowlingArm : PlayerDefaults() {

        const val RIGHT = "right"
        const val LEFT = "left"
    }

    data object BowlingType : PlayerDefaults() {

        const val SPIN = "spin"
        const val MEDIUM = "medium"
        const val FAST = "fast"
    }
}