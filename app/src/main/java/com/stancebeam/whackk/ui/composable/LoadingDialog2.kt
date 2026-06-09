package com.stancebeam.cc_lane.ui.composable

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.AlertDialogDefaults
import androidx.compose.material3.BasicAlertDialog
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.window.DialogProperties
import com.stancebeam.whackk.R
import com.stancebeam.cc_lane.ui.theme.AppTheme

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun LoadingDialog2(
    title: String,
    modifier: Modifier = Modifier,
) {
    BasicAlertDialog(
        onDismissRequest = { /*TODO*/ },
        modifier = modifier.background(
            color = AlertDialogDefaults.containerColor,
            shape = AlertDialogDefaults.shape,
        ),
        properties = DialogProperties(
            dismissOnBackPress = false,
            dismissOnClickOutside = false,
        ),
    ) {
        Column(
            modifier = Modifier.padding(24.dp),
            verticalArrangement = Arrangement.spacedBy(16.dp),
        ) {
            Text(
                text = title,
                color = AlertDialogDefaults.titleContentColor,
                style = MaterialTheme.typography.titleMedium,
            )
            Row(
                horizontalArrangement = Arrangement.spacedBy(16.dp),
                verticalAlignment = Alignment.CenterVertically,
            ) {
                CircularProgressIndicator()
                Text(
                    text = stringResource(R.string.please_wait),
                    color = AlertDialogDefaults.textContentColor,
                    style = MaterialTheme.typography.bodyMedium,
                )
            }
        }
    }
}

@Preview
@Composable
private fun LoadingDialog2Prev() {
    AppTheme {
        LoadingDialog2(
            title = "Showing loading state",
        )
    }
}