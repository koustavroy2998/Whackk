package com.stancebeam.cc_lane.ui.composable

import android.content.res.Configuration
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.stancebeam.cc_lane.data.entity.Player
import com.stancebeam.cc_lane.data.util.dummyPlayers
import com.stancebeam.cc_lane.ui.theme.AppTheme

@Composable
fun PlayerList(
    players: List<Player>,
    onClick: (Player) -> Unit,
    modifier: Modifier = Modifier,
    selectedPlayer: Player? = null,
    selectedPlayerIds: List<String> = emptyList(),
) {
    LazyColumn(
        modifier = modifier,
        contentPadding = PaddingValues(12.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp),
    ) {
        items(
            items = players,
            key = {
                it.playerId
            },
        ) {
            PlayerItem(
                value = it,
                onClick = {
                    onClick(it)
                },
                selected = it.playerId == selectedPlayer?.playerId,
                enabled = (it.playerId in selectedPlayerIds).not(),
            )
        }
    }
}

@Preview(name = "Light")
@Preview(name = "Dark", uiMode = Configuration.UI_MODE_NIGHT_YES)
@Composable
private fun PlayerListPrev() {
    AppTheme {
        PlayerList(
            players = dummyPlayers,
            onClick = {},
        )
    }
}