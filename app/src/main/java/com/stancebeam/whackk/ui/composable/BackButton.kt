package com.stancebeam.cc_lane.ui.composable

import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import com.stancebeam.whackk.R


@Composable
fun BackButton(
    onClick: () -> Unit,
    modifier: Modifier = Modifier,
) {
    IconButton(
        onClick = onClick,
        modifier = modifier,
    ) {
//        Icon(
//            imageVector = Icons.AutoMirrored.Default.ArrowBack,
//           // contentDescription = stringResource(R.string.back_arrow),
//        )
    }
}