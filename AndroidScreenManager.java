package com.yourdomain.djsugar;

import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.FrameLayout;

public class AndroidScreenManager {
    /**
     * Adjusts the view layout parameters to fit the screen properly
     * handling notch and screen cutouts on Android devices.
     */
    public static void adjustForScreenCutouts(android.view.View rootView) {
        // Get display cutout info
        android.app.Activity activity = (android.app.Activity) rootView.getContext();
        View rootView = rootView;
        
        // Get system display cutouts
        android.app.WindowMetrics metrics = activity.getWindowMetrics();
        android.graphics.Rect bounds = metrics.getBounds();
        
        // Get display cutouts
        android.graphics.Rect boundsWithCutouts = new android.graphics.Rect();
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.P) {
            boundsWithCutouts = metrics.getMaximumWindowBounds();
        }
        
        // Calculate safe area for notch
        int systemBars = 0;
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.P) {
            android.app.WindowMetrics boundsWithMinMargins = metrics.getMaximumWindowBounds();
            android.graphics.Rect boundsWithMargins = new android.graphics.Rect();
            activity.getSystemBarInsets().getTimelineInsets(boundsWithMargins);
            
            // Use insets to adjust layout
            int insetTop = boundsWithMargins.top();
            int insetLeft = boundsWithMargins.left();
            int insetRight = boundsWithMargins.right();
            int insetBottom = boundsWithMargins.bottom();
            
            // Adjust view to fit within safe area
            FrameLayout.LayoutParams params = (FrameLayout.LayoutParams) rootView.getLayoutParams();
            params.setMargins(insetLeft, insetTop, insetRight, insetBottom);
            rootView.setLayoutParams(params);
        }
        
        // Force fullscreen mode
        activity.getWindow().addFlags(android.view.WindowManager.LayoutParams.FLAG_FULLSCREEN);
        activity.getWindow().clearFlags(android.view.WindowManager.LayoutParams.FLAG_WATCH_WINDOW_OVERLAY);
    }
}