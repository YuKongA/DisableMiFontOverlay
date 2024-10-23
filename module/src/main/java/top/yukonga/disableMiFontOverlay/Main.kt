package top.yukonga.disableMiFontOverlay

import android.util.Log
import miui.util.font.FontSettings

@Suppress("unused")
object Main {
    private const val TAG = "DisableMiFontOverlay"

    @JvmStatic
    fun main() {
        runCatching {
            FontSettings::class.java
                .getDeclaredField("HAS_MIUI_VAR_FONT")
                .apply { isAccessible = true }
                .set(null, false)
        }.onFailure {
            Log.e(TAG, "Failed to replace isMiuiOptimizeEnabled return value: $it", it)
        }
    }
}