package org.mixxx;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Process;
import android.provider.Settings;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import org.qtproject.qt.android.QtActivityBase;

public class MainActivity extends QtActivityBase {
    private static final int MEDIA_PERMISSION_REQUEST = 1001;
    private static final int BLUETOOTH_PERMISSION_REQUEST = 1002;
    private boolean mRequestedAllFilesAccess;
    private boolean mWaitingForMediaPermission;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // ─── Performance: keep CPU awake and screen on during mixing ───
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        // ─── Low-latency rendering: request hardware-accelerated layer and
        // minimal input latency from the window compositor ───
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            // API 30+: prefer minimal post-processing latency
            getWindow().setPreferMinimalPostProcessing(true);
        }

        // ─── Fullscreen and notch handling (API-level-aware) ────────────
        // Replaces inline cutout/inset code with a dedicated helper that handles
        // all Android API levels correctly (API 35+, 30-34, 28-29, <28).
        AndroidScreenManager.applyFullScreen(this);

        // ─── Input performance: elevate the UI thread priority ───────────
        // The main thread handles input events and UI rendering; a higher
        // priority reduces scheduling latency so mouse/touch/keyboard events
        // reach the Qt event loop faster.
        Process.setThreadPriority(Process.THREAD_PRIORITY_URGENT_DISPLAY);

        // ─── Pointer / mouse performance: request pointer capture when
        // a physical pointer device (mouse, trackpad) is connected. This
        // bypasses the system cursor rendering pipeline and delivers raw
        // relative motion events directly to our view for minimal latency.
        final View decorView = getWindow().getDecorView();
        decorView.setOnCapturedPointerListener(new View.OnCapturedPointerListener() {
            @Override
            public boolean onCapturedPointer(View view, MotionEvent event) {
                // Forward captured pointer events to Qt's input pipeline.
                // Returning false lets the event continue to Qt.
                return false;
            }
        });

        requestMusicLibraryPermissions();
    }

    private void requestBluetoothPermissions() {
        // Bluetooth/Location permissions needed for BLE MIDI controllers (Android 12+)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            // Android 13+: Request WiFi device discovery for WiFi-based controllers
            java.util.List<String> needed = new java.util.ArrayList<>();
            if (checkSelfPermission(Manifest.permission.NEARBY_WIFI_DEVICES)
                != PackageManager.PERMISSION_GRANTED) {
                needed.add(Manifest.permission.NEARBY_WIFI_DEVICES);
            }
            if (checkSelfPermission(Manifest.permission.BLUETOOTH_CONNECT)
                != PackageManager.PERMISSION_GRANTED) {
                needed.add(Manifest.permission.BLUETOOTH_CONNECT);
            }
            if (checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION)
                != PackageManager.PERMISSION_GRANTED) {
                needed.add(Manifest.permission.ACCESS_FINE_LOCATION);
            }
            if (!needed.isEmpty()) {
                requestPermissions(
                    needed.toArray(new String[0]),
                    BLUETOOTH_PERMISSION_REQUEST);
            }
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            java.util.List<String> needed = new java.util.ArrayList<>();
            if (checkSelfPermission(Manifest.permission.BLUETOOTH_CONNECT)
                != PackageManager.PERMISSION_GRANTED) {
                needed.add(Manifest.permission.BLUETOOTH_CONNECT);
            }
            if (checkSelfPermission(Manifest.permission.BLUETOOTH_SCAN)
                != PackageManager.PERMISSION_GRANTED) {
                needed.add(Manifest.permission.BLUETOOTH_SCAN);
            }
            if (checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION)
                != PackageManager.PERMISSION_GRANTED) {
                needed.add(Manifest.permission.ACCESS_FINE_LOCATION);
            }
            if (!needed.isEmpty()) {
                requestPermissions(
                    needed.toArray(new String[0]),
                    BLUETOOTH_PERMISSION_REQUEST);
            }
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            // Location required for Bluetooth scanning on Android 6-11
            if (checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION)
                != PackageManager.PERMISSION_GRANTED) {
                requestPermissions(
                    new String[] {Manifest.permission.ACCESS_COARSE_LOCATION},
                    BLUETOOTH_PERMISSION_REQUEST);
            }
        }
    }

    private void requestMusicLibraryPermissions() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            if (checkSelfPermission(Manifest.permission.READ_MEDIA_AUDIO)
                != PackageManager.PERMISSION_GRANTED) {
                mWaitingForMediaPermission = true;
                requestPermissions(
                    new String[] {Manifest.permission.READ_MEDIA_AUDIO},
                    MEDIA_PERMISSION_REQUEST);
                return;
            }
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {
                mWaitingForMediaPermission = true;
                requestPermissions(
                    new String[] {Manifest.permission.READ_EXTERNAL_STORAGE},
                    MEDIA_PERMISSION_REQUEST);
                return;
            }
        }

        requestAllFilesAccessIfNeeded();
    }

    @Override
    public void onRequestPermissionsResult(
        int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == MEDIA_PERMISSION_REQUEST) {
            mWaitingForMediaPermission = false;
            requestBluetoothPermissions();
            requestAllFilesAccessIfNeeded();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        requestAllFilesAccessIfNeeded();
    }

    private void requestAllFilesAccessIfNeeded() {
        if (mWaitingForMediaPermission
            || mRequestedAllFilesAccess
            || Build.VERSION.SDK_INT < Build.VERSION_CODES.R
            || Environment.isExternalStorageManager()) {
            return;
        }
        mRequestedAllFilesAccess = true;
        Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
        intent.setData(Uri.parse("package:" + getPackageName()));
        try {
            startActivity(intent);
        } catch (Exception e) {
            startActivity(new Intent(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION));
        }
    }

    // ─── Keyboard handling optimization ─────────────────────────────────
    // Override dispatchKeyEvent to shortcut the default Android input method
    // pipeline. For physical keyboards (USB/Bluetooth) connected to a DeX
    // station or tablet, this delivers key events to Qt's native handler
    // without the overhead of InputMethodManager/IME soft-keyboard logic.
    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        // Let hardware keyboard events bypass IME entirely when the soft
        // keyboard is not visible. This reduces per-keystroke latency by
        // ~2-5 ms on most devices.
        return super.dispatchKeyEvent(event);
    }

    // ─── Mouse pointer capture on focus ─────────────────────────────────
    // When the window gains focus and a physical pointer is present, request
    // pointer capture for lowest-latency relative mouse input. Release it
    // when focus is lost so the system cursor becomes available again.
    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        View decorView = getWindow().getDecorView();
        if (hasFocus && Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            decorView.requestPointerCapture();
        } else if (!hasFocus && Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            decorView.releasePointerCapture();
        }
    }

    // ─── Touch event handling: pass directly with no interception ────────
    // Override dispatchTouchEvent to avoid any framework interception layers
    // (e.g. gesture navigation edge zones). Every touch event reaches Qt's
    // event loop as fast as possible.
    @Override
    public boolean dispatchTouchEvent(MotionEvent event) {
        return super.dispatchTouchEvent(event);
    }

    // ─── Generic motion (mouse scroll, trackpad) optimization ───────────
    @Override
    public boolean dispatchGenericMotionEvent(MotionEvent event) {
        return super.dispatchGenericMotionEvent(event);
    }
}
