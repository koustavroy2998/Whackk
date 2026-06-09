package com.stancebeam.cc_lane.ui.composable

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.SheetState
import androidx.compose.material3.Text
import androidx.compose.material3.rememberModalBottomSheetState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.stancebeam.whackk.R
import com.stancebeam.cc_lane.data.bluetooth.StanceBeamDevice
import com.stancebeam.cc_lane.data.entity.Player
import kotlinx.coroutines.launch

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SelectPlayerSheet(
    device: StanceBeamDevice?,
    players: List<Player>,
    selected: Player?,
    onDismiss: () -> Unit,
    onSelect: (StanceBeamDevice, Player) -> Unit,
    modifier: Modifier = Modifier,
    sheetState: SheetState = rememberModalBottomSheetState(
        skipPartiallyExpanded = true,
    ),
    selectedPlayerIds: List<String> = emptyList(),
    onRefreshPlayers: () -> Unit = {},
) {
    val coroutineScope = rememberCoroutineScope()
    if (device != null) {
        ModalBottomSheet(
            onDismissRequest = onDismiss,
            modifier = modifier,
            sheetState = sheetState,
        ) {
            Text(
                text = device.name ?: stringResource(R.string.unknown),
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(
                        horizontal = 12.dp,
                    ),
                style = MaterialTheme.typography.titleMedium,
            )
            Text(
                text = device.address,
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(
                        horizontal = 12.dp,
                    ),
                style = MaterialTheme.typography.bodyMedium,
            )
            if (players.isEmpty()) {
                Column(
                    modifier = Modifier
                        .fillMaxWidth()
                        .weight(1f),
                    verticalArrangement = Arrangement.Center,
                    horizontalAlignment = Alignment.CenterHorizontally,
                ) {
                    Text(
                        text = stringResource(R.string.no_players),
                    )
                    Button(
                        onClick = onRefreshPlayers,
                        contentPadding = ButtonDefaults.ButtonWithIconContentPadding,
                    ) {
                        Icon(
                            imageVector = Icons.Default.Refresh,
                            contentDescription = null,
                            modifier = Modifier.size(ButtonDefaults.IconSize),
                        )
                        Spacer(
                            modifier = Modifier.size(ButtonDefaults.IconSpacing),
                        )
                        Text(
                            text = stringResource(R.string.refresh),
                        )
                    }
                }
            } else {
                PlayerList(
                    players = players,
                    onClick = { player ->
                        coroutineScope.launch { sheetState.hide() }
                            .invokeOnCompletion {
                                onSelect(device, player)
                            }
                    },
                    modifier = Modifier
                        .fillMaxWidth()
                        .weight(1f),
                    selectedPlayer = selected,
                    selectedPlayerIds = selectedPlayerIds,
                )
            }
        }
    }
}