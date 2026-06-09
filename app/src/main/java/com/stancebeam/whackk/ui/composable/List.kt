package com.stancebeam.cc_lane.ui.composable

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyListScope
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.shadow
import androidx.compose.ui.graphics.Shape
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.stancebeam.cc_lane.ui.theme.AppTheme
import com.stancebeam.cc_lane.util.UiText

@Composable
fun ListHeaderItem(
    text: String,
    modifier: Modifier = Modifier,
) {
    Text(
        text = text,
        modifier = modifier.padding(
            horizontal = 16.dp,
        ),
        style = MaterialTheme.typography.titleMedium,
    )
}

@Composable
fun ListMessageItem(
    text: String,
    modifier: Modifier = Modifier,
    shape: Shape = MaterialTheme.shapes.small,
) {
    Text(
        text = text,
        modifier = modifier
            .shadow(
                elevation = 1.dp,
                shape = shape,
            )
            .background(
                color = MaterialTheme.colorScheme.surfaceVariant,
                shape = shape,
            )
            .padding(
                horizontal = 16.dp,
                vertical = 8.dp,
            ),
        textAlign = TextAlign.Center,
        style = MaterialTheme.typography.bodySmall,
    )
}

fun LazyListScope.headerItem(
    text: UiText,
    modifier: Modifier = Modifier,
    key: Any? = null,
    contentType: Any? = null,
) {
    item(
        key = key,
        contentType = contentType,
    ) {
        ListHeaderItem(
            text = text.asString(),
            modifier = modifier.fillParentMaxWidth(),
        )
    }
}

fun LazyListScope.messageItem(
    text: UiText,
    modifier: Modifier = Modifier,
    key: Any? = null,
    contentType: Any? = null,
) {
    item(
        key = key,
        contentType = contentType,
    ) {
        ListMessageItem(
            text = text.asString(),
            modifier = modifier.fillParentMaxWidth(),
        )
    }
}

@Preview(showBackground = true)
@Composable
private fun ListPreview() {
    AppTheme {
        Column(
            modifier = Modifier.padding(12.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp),
        ) {
            ListHeaderItem("Header")
            ListMessageItem("This is a message.")
        }
    }
}