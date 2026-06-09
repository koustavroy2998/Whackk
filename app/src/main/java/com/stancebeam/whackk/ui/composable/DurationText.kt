package com.stancebeam.cc_lane.ui.composable

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.aspectRatio
import androidx.compose.foundation.layout.height
import androidx.compose.material3.Icon
import androidx.compose.material3.LocalTextStyle
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.stancebeam.whackk.R
import com.stancebeam.cc_lane.ui.theme.AppTheme

@Composable
fun DurationText(
    value: Int,
    modifier: Modifier = Modifier,
    textStyle: TextStyle = LocalTextStyle.current,
) {
    var hours by remember {
        mutableIntStateOf(0)
    }
    var minutes by remember {
        mutableIntStateOf(0)
    }
    var seconds by remember {
        mutableIntStateOf(0)
    }
    LaunchedEffect(value) {
        hours = value / 3600
        minutes = (value % 3600) / 60
        seconds = value % 60
    }
    Row(
        modifier = modifier,
        horizontalArrangement = Arrangement.spacedBy(
            space = 4.dp,
            alignment = Alignment.CenterHorizontally,
        ),
        verticalAlignment = Alignment.CenterVertically,
    ) {
        Icon(
            painter = painterResource(R.drawable.ic_outline_schedule),
            contentDescription = null,
            modifier = Modifier
                .height(20.dp)
                .aspectRatio(1f / 1f),
        )
        Text(
            text = if (hours > 0) {
                stringResource(R.string.hh_mm_ss, hours, minutes, seconds)
            } else {
                stringResource(R.string.mm_ss, minutes, seconds)
            },
            style = textStyle,
        )
    }
}

@Preview(showBackground = true)
@Composable
private fun DurationTextPreview() {
    AppTheme {
        Column {
            DurationText(
                value = 3599,
            )
            DurationText(
                value = 7199,
            )
        }
    }
}