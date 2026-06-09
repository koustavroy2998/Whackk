package com.stancebeam.cc_lane.ui.composable

import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.DateRange
import androidx.compose.material3.DatePicker
import androidx.compose.material3.DatePickerDialog
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.rememberDatePickerState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource

import com.stancebeam.cc_lane.util.formatTimestamp
import com.stancebeam.whackk.R

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun DateTextField(
    label: String,
    value: Long?,
    onValueChange: (Long) -> Unit,
    modifier: Modifier = Modifier,
) {
    val datePickerState = rememberDatePickerState()
    var showDatePicker by remember {
        mutableStateOf(false)
    }
    val date = value?.let { formatTimestamp(it,"dd/MM/yyyy") } ?: ""
    OutlinedTextField(
        value = date,
        onValueChange = {},
        modifier = modifier.fillMaxWidth(),
        readOnly = true,
        label = {
            Text(label)
        },
        trailingIcon = {
            IconButton(
                onClick = { showDatePicker = true },
            ) {
                Icon(
                    imageVector = Icons.Default.DateRange,
                    contentDescription = "Select date",
                )
            }
        },
    )
    if (showDatePicker) {
        DatePickerDialog(
            onDismissRequest = { showDatePicker = false },
            confirmButton = {
                TextButton(
                    onClick = {
                        datePickerState.selectedDateMillis?.let {
                            onValueChange(it)
                            showDatePicker = false
                        }
                    },
                ) {
                    Text(
                        text = stringResource(R.string.ok),
                    )
                }
            },
            dismissButton = {
                TextButton(
                    onClick = { showDatePicker = false },
                ) {
                    Text(
                        text = stringResource(R.string.cancel),
                    )
                }
            },
        ) {
            DatePicker(datePickerState)
        }
    }
}