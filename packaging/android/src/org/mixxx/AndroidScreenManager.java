package org.mixxx;

import android.app.Activity;
import android.os.Build;
import android.view.View;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.view.WindowManager;

/**
 * Helper class to adjust the app window for devices with display cutouts (notches).
 * It ensures the UI fills the full screen while respecting safe insets.
 *
 * On API 35+ (Android 15), ALL the legacy fullscreen APIs are deprecated
 * (FLAG_FULLSCREEN, setSystemUiVisibility, SYSTEM_UI_FLAG_*, and even
 * setDecorFitsSystemWindows). Only WindowInsetsController remains non-deprecated.
 * We split into separate API-level paths and suppress deprecation warnings
 * on the backwards-compatible branches — they never execute on API 35+.
 */
@SuppressWarnings("deprecation")
public class AndroidScreenManager {
    public static void applyFullScreen(Activity activity) {
        if (activity == null)
            return;

        if (Build.VERSION.SDK_INT >= 35) {
            // Android 15+ (API 35+): use non-deprecated insets controller only.
            // setDecorFitsSystemWindows is also deprecated on this level.
            WindowInsetsController controller = activity.getWindow().getInsetsController();
            if (controller != null) {
                controller.hide(WindowInsets.Type.systemBars());
                controller.setSystemBarsBehavior(
                    WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
            }
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            // Android 11-14 (API 30-34): insets controller + setDecorFitsSystemWindows
            // (not deprecated at these levels).
            activity.getWindow().setDecorFitsSystemWindows(false);
            WindowInsetsController controller = activity.getWindow().getInsetsController();
            if (controller != null) {
                controller.hide(WindowInsets.Type.systemBars());
                controller.setSystemBarsBehavior(
                    WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
            }
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            // Android 9-10 (API 28-29): use system UI flags (not deprecated here).
            activity.getWindow().getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
        } else {
            // Android < 9: FLAG_FULLSCREEN (not deprecated at these levels).
            activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        }

        // Allow layout behind display cutouts (notch) — API 28+.
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            activity.getWindow().getAttributes().layoutInDisplayCutoutMode =
                WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
        }
    }
}
