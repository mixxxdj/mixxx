package org.mixxx;

import android.app.Activity;
import android.os.Build;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.view.WindowManager;

/**
 * Helper class to adjust the app window for devices with display cutouts (notches).
 * It ensures the UI fills the full screen while respecting safe insets.
 */
public class AndroidScreenManager {
    public static void applyFullScreen(Activity activity) {
        if (activity == null) return;
        // Enable full-screen mode
        activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        // Allow layout behind cutout
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            activity.getWindow().getAttributes().layoutInDisplayCutoutMode =
                    WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
        }
        // Apply inset controller to hide system bars and use safe area
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            WindowInsetsController controller = activity.getWindow().getInsetsController();
            if (controller != null) {
                controller.hide(WindowInsets.Type.systemBars());
                controller.setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
            }
        }
    }
}
