package com.stancebeam.cc_lane.ui.composable

import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.ColorFilter
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.constraintlayout.compose.ConstraintLayout
import androidx.constraintlayout.compose.Dimension
import coil.compose.AsyncImage
import com.stancebeam.whackk.R
import com.stancebeam.cc_lane.data.entity.Player
import com.stancebeam.cc_lane.data.util.PlayerDefaults
import com.stancebeam.cc_lane.data.util.dummyPlayers

@Composable
fun PlayerImage(
    url: String?,
    gender: String,
    modifier: Modifier = Modifier,
) {
    val commonModifier = modifier
        .size(48.dp)
        .clip(CircleShape)
    if (url != null) {
        AsyncImage(
            model = url,
            contentDescription = null,
            modifier = commonModifier,
            contentScale = ContentScale.Crop,
        )
    } else {
        Box(
            modifier = commonModifier.background(MaterialTheme.colorScheme.surface),
            contentAlignment = Alignment.Center,
        ) {
            Image(
                painter = if (gender == PlayerDefaults.Gender.MALE) {
                    painterResource(R.drawable.ic_person)
                } else {
                    painterResource(R.drawable.ic_person_2)
                },
                contentDescription = null,
                colorFilter = ColorFilter.tint(MaterialTheme.colorScheme.onSurface),
            )
        }
    }
}

@Composable
fun PlayerItem(
    value: Player,
    onClick: () -> Unit,
    modifier: Modifier = Modifier,
    selected: Boolean = false,
    enabled: Boolean = true,
) {
    Card(
        onClick = onClick,
        modifier = modifier,
        enabled = selected || enabled,
        colors = CardDefaults.cardColors(
            containerColor = if (selected) {
                MaterialTheme.colorScheme.inversePrimary
            } else {
                MaterialTheme.colorScheme.surfaceVariant
            },
        ),
    ) {
        ConstraintLayout(
            modifier = Modifier.fillMaxWidth(),
        ) {
            val (imageRef, columnRef) = createRefs()
            PlayerImage(
                url = value.profilePicture,
                gender = value.gender ?: PlayerDefaults.Gender.MALE,
                modifier = Modifier.constrainAs(imageRef) {
                    top.linkTo(parent.top, margin = 16.dp)
                    bottom.linkTo(parent.bottom, margin = 16.dp)
                    start.linkTo(parent.start, margin = 16.dp)
                },
            )
            Column(
                modifier = Modifier.constrainAs(columnRef) {
                    top.linkTo(imageRef.top)
                    bottom.linkTo(imageRef.bottom)
                    start.linkTo(imageRef.end, margin = 16.dp)
                    end.linkTo(parent.end, margin = 16.dp)
                    width = Dimension.fillToConstraints
                },
            ) {
                Text(
                    text = value.fullName,
                    modifier = Modifier.fillMaxWidth(),
                    style = MaterialTheme.typography.titleMedium,
                )
                Text(
                    text = value.email,
                    modifier = Modifier.fillMaxWidth(),
                    style = MaterialTheme.typography.bodyMedium,
                )
            }
        }
    }
}

@Preview(name = "Light", showBackground = true)
@Composable
private fun PlayerItemPreview() {
    Column(
        modifier = Modifier.padding(12.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp),
    ) {
        PlayerItem(
            value = dummyPlayers[0],
            onClick = {},
        )
        PlayerItem(
            value = dummyPlayers[1],
            onClick = {},
            selected = true,
        )
    }
}