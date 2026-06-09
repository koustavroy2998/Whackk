package com.stancebeam.cc_lane.ui.composable

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Person
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Card
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp

import com.stancebeam.cc_lane.data.bluetooth.StanceBeamDevice
import com.stancebeam.cc_lane.data.entity.Player
import com.stancebeam.cc_lane.ui.theme.AppTheme
import com.stancebeam.whackk.R

private val emptyFunction = {}

@Composable
fun DeviceItem(
    value: StanceBeamDevice,
    modifier: Modifier = Modifier,
    onConnect: () -> Unit = emptyFunction,
    onDisconnect: () -> Unit = emptyFunction,
    onSelectPlayer: () -> Unit = emptyFunction,
    player: Player? = null,
) {
    Card(onConnect) {
        Column(
            modifier = modifier
                .fillMaxWidth()
                .padding(16.dp),
        ) {
            Text(
                text = value.name ?: stringResource(R.string.unknown),
                modifier = modifier.fillMaxWidth(),
                style = MaterialTheme.typography.titleMedium,
            )
            Text(
                text = value.address,
                modifier = modifier.fillMaxWidth(),
                style = MaterialTheme.typography.bodyMedium,
            )
            if (onDisconnect != emptyFunction || onSelectPlayer != emptyFunction) {
                Spacer(
                    modifier = Modifier.size(8.dp),
                )
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp),
                    verticalAlignment = Alignment.CenterVertically,
                ) {
                    if (onDisconnect != emptyFunction) {
                        OutlinedButton(
                            onClick = onDisconnect,
                        ) {
                            Text(
                                text = stringResource(R.string.disconnect),
                            )
                        }
                    }
                    if (onSelectPlayer != emptyFunction) {
                        OutlinedButton(
                            onClick = onSelectPlayer,
                            contentPadding = ButtonDefaults.ButtonWithIconContentPadding,
                        ) {
                            Icon(
                                imageVector = Icons.Default.Person,
                                contentDescription = null,
                                modifier = Modifier.size(ButtonDefaults.IconSize),
                            )
                            Spacer(
                                modifier = Modifier.size(ButtonDefaults.IconSpacing),
                            )
                            Text(
                                text = player?.fullName ?: stringResource(R.string.select_player),
                            )
                        }
                    }
                }
            }
        }
    }
}

@Preview(name = "Light")
@Composable
private fun DeviceItemPreview() {
    AppTheme {
        Column(
            modifier = Modifier.padding(12.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp),
        ) {
            DeviceItem(
                value = StanceBeamDevice(
                    name = "Test Device",
                    address = "00:00:00:00:00:00",
                ),
                onConnect = {},
            )
            DeviceItem(
                value = StanceBeamDevice(
                    name = "Test Device",
                    address = "00:00:00:00:00:00",
                ),
                onDisconnect = {},
            )
            DeviceItem(
                value = StanceBeamDevice(
                    name = "Test Device",
                    address = "00:00:00:00:00:00",
                ),
                onSelectPlayer = {},
            )
        }
    }
}