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
 * FLAG_FULLSCREEN is deprecated on API 35+ (Android 15) and produces a compile-time
 * warning. We avoid it entirely on API 30+ by using setDecorFitsSystemWindows(false)
 * + WindowInsetsController. On API 28-29 we use the legacy SYSTEM_UI_FLAG_FULLSCREEN
 * approach. FLAG_FULLSCREEN is only kept as a fallback for pre-9 devices that will
 * never be compiled against SDK 35.
 */
public class AndroidScreenManager {
    public static void applyFullScreen(Activity activity) {
        if (activity == null) return;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            // Android 11+ (API 30+): proper insets-based fullscreen.
            // Draw behind system bars so content extends into notch area.
            activity.getWindow().setDecorFitsSystemWindows(false);
            WindowInsetsController controller = activity.getWindow().getInsetsController();
            if (controller != null) {
                controller.hide(WindowInsets.Type.systemBars());
                controller.setSystemBarsBehavior(
                        WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
            }
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            // Android 9-10 (API 28-29): use legacy system UI flags.
            activity.getWindow().getDecorView().setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_FULLSCREEN
                    | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                    | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
        } else {
            // Android < 9: deprecated on API 35+ but this branch is never
            // reached on devices that new — kept for backward compatibility.
            activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        }

        // Allow layout behind display cutouts (notch) — API 28+.
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            activity.getWindow().getAttributes().layoutInDisplayCutoutMode =
                    WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
        }
    }
}