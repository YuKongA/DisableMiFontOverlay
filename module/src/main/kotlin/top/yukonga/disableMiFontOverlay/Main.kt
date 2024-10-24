package top.yukonga.disableMiFontOverlay

import android.util.Log
import miui.util.font.FontSettings

@Suppress("unused")
object Main {
    private const val TAG = "DisableMiFontOverlay"

    @JvmStatic
    fun init() {
        runCatching {
            FontSettings::class.java
                .getDeclaredField("HAS_MIUI_VAR_FONT")
                .let { field ->
                    field.isAccessible = true
                    field.set(null, false)
                }
        }.onFailure {
            Log.e(TAG, "Failed to replace isMiuiOptimizeEnabled return value: $it", it)
        }
    }
}