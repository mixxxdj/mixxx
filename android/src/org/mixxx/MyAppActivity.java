package org.mixxx;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.media.AudioDeviceInfo;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Debug;
import android.os.PowerManager;
import android.util.Log;
import android.widget.Toast;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import org.qtproject.qt.android.bindings.QtActivity;

public class MyAppActivity extends QtActivity {
    private static MyAppActivity s_activity = null;
    private static final String ACTION_USB_PERMISSION =
        "org.mixxx.permissions.USB_PERMISSION";
    private static final String TAG = "MixxxMainActivity";
    private UsbManager usbManager;
    private PendingIntent mPermissionIntent;

    // Keep connection alive
    private List<UsbDeviceConnection> usbDevices = new ArrayList<UsbDeviceConnection>();
    private int usbDevicesCount = 0;
    private PowerManager.WakeLock wakeLock;

    private static native void registerUsbDevice(int usbDeviceFileDescriptor);
    private static native void finishUsbRegistering();

    private void handleUsbDeviceConnection(UsbDeviceConnection connection) {
        registerUsbDevice(connection.getFileDescriptor());
        usbDevices.add(connection);
        if (usbDevices.size() == usbDevicesCount) {
            finishUsbRegistering();
        }
    }

    public static void listAudioDevices(AudioManager audioManager) {
        if (audioManager == null) {
            Log.e(TAG, "AudioManager is null");
            return;
        }

        // Get all output devices
        AudioDeviceInfo[] outputDevices = audioManager.getDevices(AudioManager.GET_DEVICES_OUTPUTS);
        Log.d(TAG, "=== OUTPUT DEVICES ===");
        for (AudioDeviceInfo device : outputDevices) {
            logDeviceInfo(device);
        }

        // Get all input devices
        AudioDeviceInfo[] inputDevices = audioManager.getDevices(AudioManager.GET_DEVICES_INPUTS);
        Log.d(TAG, "=== INPUT DEVICES ===");
        for (AudioDeviceInfo device : inputDevices) {
            logDeviceInfo(device);
        }
    }

    private static void logDeviceInfo(AudioDeviceInfo device) {
        if (device == null) {
            Log.w(TAG, "Device info is null");
            return;
        }

        StringBuilder sb = new StringBuilder();
        sb.append("Type: ").append(getDeviceTypeName(device.getType()));
        sb.append(", Product Name: ").append(device.getProductName() != null ? device.getProductName().toString() : "N/A");
        sb.append(", ID: ").append(device.getId());
        sb.append(", Channels: ").append(getIntArrayString(device.getChannelCounts()));
        sb.append(", Channel masks: ").append(getIntArrayString(device.getChannelIndexMasks()));
        sb.append(", Channels: ").append(getChannelMaskString(device.getChannelMasks()));
        sb.append(", Sample Rates: ").append(getSampleRatesString(device.getSampleRates()));

        Log.d(TAG, sb.toString());
    }

    private static String getDeviceTypeName(int type) {
        switch (type) {
            case AudioDeviceInfo.TYPE_BUILTIN_EARPIECE:
                return "BUILTIN_EARPIECE";
            case AudioDeviceInfo.TYPE_BUILTIN_SPEAKER:
                return "BUILTIN_SPEAKER";
            case AudioDeviceInfo.TYPE_WIRED_HEADSET:
                return "WIRED_HEADSET";
            case AudioDeviceInfo.TYPE_BLUETOOTH_A2DP:
                return "BLUETOOTH_A2DP";
            case AudioDeviceInfo.TYPE_BLUETOOTH_SCO:
                return "BLUETOOTH_SCO";
            case AudioDeviceInfo.TYPE_USB_DEVICE:
                return "USB_DEVICE";
            case AudioDeviceInfo.TYPE_USB_HEADSET:
                return "USB_HEADSET";
            case AudioDeviceInfo.TYPE_HDMI:
                return "HDMI";
            case AudioDeviceInfo.TYPE_HDMI_ARC:
                return "HDMI_ARC";
            case AudioDeviceInfo.TYPE_DOCK:
                return "DOCK";
            case AudioDeviceInfo.TYPE_FM:
                return "FM";
            case AudioDeviceInfo.TYPE_BUILTIN_MIC:
                return "BUILTIN_MIC";
            case AudioDeviceInfo.TYPE_FM_TUNER:
                return "FM_TUNER";
            case AudioDeviceInfo.TYPE_TV_TUNER:
                return "TV_TUNER";
            case AudioDeviceInfo.TYPE_TELEPHONY:
                return "TELEPHONY";
            case AudioDeviceInfo.TYPE_AUX_LINE:
                return "AUX_LINE";
            case AudioDeviceInfo.TYPE_IP:
                return "IP";
            case AudioDeviceInfo.TYPE_BUS:
                return "BUS";
            case AudioDeviceInfo.TYPE_USB_ACCESSORY:
                return "USB_ACCESSORY";
            case AudioDeviceInfo.TYPE_BLE_HEADSET:
                return "BLE_HEADSET";
            case AudioDeviceInfo.TYPE_BLE_SPEAKER:
                return "BLE_SPEAKER";
            case AudioDeviceInfo.TYPE_HEARING_AID:
                return "HEARING_AID";
            case AudioDeviceInfo.TYPE_REMOTE_SUBMIX:
                return "REMOTE_SUBMIX";
            case AudioDeviceInfo.TYPE_UNKNOWN:
                return "UNKNOWN";
            default:
                return "UNKNOWN_TYPE";
        }
    }
    private static String getIntArrayString(int[] values) {
        StringBuilder sb = new StringBuilder();
        for (int value : values) {
            sb.append(value);
            sb.append(", ");
        }
        if (sb.length() != 0) {
            sb.setLength(sb.length() - 2); // Remove trailing comma
        }
        return sb.toString();
    }

    private static String getChannelMaskString(int[] channelMasks) {
        StringBuilder sb = new StringBuilder();
        for (int channelMask : channelMasks) {
            switch (channelMask) {
                case AudioFormat.CHANNEL_OUT_MONO:
                    sb.append("MONO, ");
                case AudioFormat.CHANNEL_OUT_STEREO:
                    sb.append("STEREO, ");
                case AudioFormat.CHANNEL_OUT_QUAD:
                    sb.append("QUAD, ");
                case AudioFormat.CHANNEL_OUT_5POINT1:
                    sb.append("5.1, ");
                case AudioFormat.CHANNEL_OUT_7POINT1:
                    sb.append("7.1, ");
                default:
                    sb.append("UNKNOWN_CHANNEL_MASK, ");
            }
        }
        if (sb.length() != 0) {
            sb.setLength(sb.length() - 2); // Remove trailing comma
        }
        return sb.toString();
    }

    private static String getSampleRatesString(int[] sampleRates) {
        if (sampleRates == null || sampleRates.length == 0) {
            return "N/A";
        }
        StringBuilder sb = new StringBuilder();
        for (int rate : sampleRates) {
            sb.append(rate).append("Hz, ");
        }
        if (sb.length() != 0) {
            sb.setLength(sb.length() - 2); // Remove trailing comma
        }
        return sb.toString();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        AudioManager audioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        listAudioDevices(audioManager);
        super.onCreate(savedInstanceState);
        s_activity = this;
        Log.i(TAG, "Connecting to USB devices");

        usbManager = (UsbManager) getSystemService(Context.USB_SERVICE);

        // Create a PendingIntent for USB permission request
        mPermissionIntent = PendingIntent.getBroadcast(this, 0,
            new Intent(ACTION_USB_PERMISSION), PendingIntent.FLAG_IMMUTABLE);

        IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
        // filter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
        // filter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
        registerReceiver(usbPermissionReceiver, filter, RECEIVER_NOT_EXPORTED);

        listUsbDevices();
        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
        wakeLock = pm.newWakeLock(PowerManager.FULL_WAKE_LOCK, "Mixxx");
        wakeLock.acquire();
    }

    private void listUsbDevices() {
        HashMap<String, UsbDevice> deviceList = usbManager.getDeviceList();
        usbDevicesCount = deviceList.size();
        if (deviceList.isEmpty()) {
            Log.i(TAG, "No USB devices connected.");
            return;
        }

        for (UsbDevice usbDevice : deviceList.values()) {
            StringBuffer builder = new StringBuffer();
            builder
                .append("Product Name: " + usbDevice.getProductName())
                .append(", Product ID: " + Integer.toHexString(usbDevice.getProductId()))
                .append(", Manufacturer Name: " + usbDevice.getManufacturerName())
                .append(", Vendor ID: " + Integer.toHexString(usbDevice.getVendorId()))
                .append(", Device Name: " + usbDevice.getDeviceName())
                .append(", Device Protocol: " + usbDevice.getDeviceProtocol())
                .append(", Device Class: " + usbDevice.getDeviceClass())
                .append(", Device Subclass: " + usbDevice.getDeviceSubclass())
                .append(", Version: " + usbDevice.getVersion());
            Log.d(TAG, builder.toString());
            // Request permission for each device
            if (usbManager.hasPermission(usbDevice)) {
                Log.d(TAG, "Already have permission");
                UsbDeviceConnection usbDeviceConnection = usbManager.openDevice(usbDevice);
                Log.d(TAG, "Open device " + usbDevice.getDeviceName() + " as " + usbDeviceConnection.getFileDescriptor());
                handleUsbDeviceConnection(usbDeviceConnection);
            } else {
                Log.d(TAG, "Requesting permission to " + usbDevice.getDeviceName());
                usbManager.requestPermission(usbDevice, mPermissionIntent);
                Log.d(TAG, "Permission requested");
            }
        }
    }

    private final BroadcastReceiver usbPermissionReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "Received " + action);
            if (ACTION_USB_PERMISSION.equals(action)) {
                synchronized (this) {
                    UsbDevice usbDevice = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                    if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                        if (usbDevice != null) {
                            Toast.makeText(context, "Permission granted for " + usbDevice.getDeviceName(),
                                     Toast.LENGTH_SHORT)
                                .show();
                            UsbDeviceConnection usbDeviceConnection = usbManager.openDevice(usbDevice);
                            handleUsbDeviceConnection(usbDeviceConnection);
                            Toast.makeText(context, "Open device " + usbDevice.getDeviceName() + " as " + usbDeviceConnection.getFileDescriptor(),
                                     Toast.LENGTH_SHORT)
                                .show();
                        }
                    } else {
                        Toast.makeText(context, "Permission denied for " + usbDevice.getDeviceName(),
                                 Toast.LENGTH_SHORT)
                            .show();
                    }
                }
            }
        }
    };

    @Override
    protected void onDestroy() {
        wakeLock.release();
        super.onDestroy();
        s_activity = null;
    }
}
