package org.mixxx;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;
import android.util.Log;

public class UsbPermission {
    private static final String ACTION_USB_PERMISSION =
        "org.mixxx.permissions.USB_PERMISSION";
    private static final String TAG = "MixxxUsbPermission";
    private static native void usbDeviceAccessResult(Object device, boolean granted);
    public boolean registerServiceBroadcastReceiver(Context context) {
        try {
            IntentFilter intentFilter = new IntentFilter(ACTION_USB_PERMISSION);
            context.registerReceiver(usbPermissionReceiver, intentFilter, Context.RECEIVER_NOT_EXPORTED);
            Log.i(TAG, "Registered broadcast receiver");
            return true;
        } catch (Exception e) {
            Log.w(TAG, "Unable to register the broadcast receiver: " + e.toString());
            return false;
        }
    }

    private final BroadcastReceiver usbPermissionReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.v(TAG, "Received " + action);
            if (ACTION_USB_PERMISSION.equals(action)) {
                synchronized (this) {
                    UsbDevice usbDevice = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                    if (usbDevice == null) {
                        Log.e(TAG, "USB device is null");
                        return;
                    }
                    boolean granted = intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false);
                    usbDeviceAccessResult(usbDevice, granted);
                    if (!granted) {
                        Log.w(TAG, "Permission was denied");
                    } else {
                        Log.i(TAG, "Permission was granted");
                    }
                }
            }
        }
    };
}
