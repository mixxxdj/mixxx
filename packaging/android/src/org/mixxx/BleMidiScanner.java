package org.mixxx;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.BluetoothLeScanner;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.ParcelUuid;
import android.util.Log;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

import androidx.core.content.ContextCompat;

/**
 * BLE MIDI device scanner for Mixxx.
 *
 * Scans for BLE devices advertising the standard BLE MIDI service UUID
 * (03B80E5A-EDE8-4B33-A751-6CE34EC4C700) and provides discovered device
 * information to the C++ layer via JNI.
 *
 * This class is instantiated and called from the C++ side through Qt JNI
 * (BleMidiEnumerator). All public methods invoked from C++ are static.
 */
public class BleMidiScanner {
    private static final String TAG = "MixxxBleMidiScanner";

    // Standard BLE MIDI Service UUID defined by the MIDI specification
    private static final UUID BLE_MIDI_SERVICE_UUID =
            UUID.fromString("03B80E5A-EDE8-4B33-A751-6CE34EC4C700");

    // Maps BluetoothDevice address -> device name (null if unavailable)
    private static final Map<String, String> sDiscoveredDevices = new HashMap<>();

    // Tracks whether a scan is currently in progress
    private static volatile boolean sScanning = false;

    // Tracks whether we have an active BLE connection
    private static volatile boolean sConnected = false;

    // The BLE scanner handle; null when not scanning
    private static BluetoothLeScanner sLeScanner = null;

    // Singleton scan callback
    private static final ScanCallback sScanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            if (result == null) {
                return;
            }
            BluetoothDevice device = result.getDevice();
            String address = device.getAddress();
            String name = null;
            try {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                    // Android 13+: name access requires BLUETOOTH_CONNECT (already checked)
                    if (ContextCompat.checkSelfPermission(
                            getContext(), Manifest.permission.BLUETOOTH_CONNECT)
                            == PackageManager.PERMISSION_GRANTED) {
                        name = device.getName();
                    }
                } else {
                    name = device.getName();
                }
            } catch (SecurityException e) {
                Log.w(TAG, "Permission denied reading device name for " + address);
            }

            if (name == null || name.isEmpty()) {
                name = "Unknown BLE MIDI Device";
            }

            synchronized (sDiscoveredDevices) {
                if (!sDiscoveredDevices.containsKey(address)) {
                    sDiscoveredDevices.put(address, name);
                    Log.i(TAG, "Discovered BLE MIDI device: " + name + " [" + address + "]");
                }
            }
        }

        @Override
        public void onScanFailed(int errorCode) {
            Log.e(TAG, "BLE scan failed with error code: " + errorCode);
            sScanning = false;
        }
    };

    // Cached context reference (the launching Activity) for permission checks
    private static Context sContext;

    /**
     * Start a BLE scan for devices advertising the MIDI service.
     *
     * Called from C++ via JNI as:
     *   static boolean startScan(Context context, String serviceUuid)
     *
     * @param context The Android context (typically the Activity)
     * @param serviceUuid The service UUID to scan for (e.g. "03B80E5A-EDE8-4B33-A751-6CE34EC4C700")
     * @return true if the scan was started successfully, false otherwise
     */
    public static boolean startScan(Context context, String serviceUuid) {
        if (context == null) {
            Log.e(TAG, "Cannot start BLE scan: context is null");
            return false;
        }

        sContext = context.getApplicationContext();

        // Stop any existing scan before starting a new one
        stopScan();

        // Verify Bluetooth is available and enabled
        BluetoothManager bluetoothManager =
                (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        if (bluetoothManager == null) {
            Log.e(TAG, "Bluetooth not supported on this device");
            return false;
        }

        BluetoothAdapter bluetoothAdapter = bluetoothManager.getAdapter();
        if (bluetoothAdapter == null || !bluetoothAdapter.isEnabled()) {
            Log.e(TAG, "Bluetooth is not enabled");
            return false;
        }

        // Check required permissions based on Android version
        if (!hasRequiredPermissions(context)) {
            Log.e(TAG, "Missing required Bluetooth permissions for BLE scan");
            return false;
        }

        // Get the BLE scanner
        sLeScanner = bluetoothAdapter.getBluetoothLeScanner();
        if (sLeScanner == null) {
            Log.e(TAG, "BluetoothLeScanner not available");
            return false;
        }

        // Build scan filter for the BLE MIDI service
        UUID serviceUuidParsed;
        try {
            serviceUuidParsed = UUID.fromString(serviceUuid);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "Invalid service UUID: " + serviceUuid);
            // Fall back to standard MIDI UUID
            serviceUuidParsed = BLE_MIDI_SERVICE_UUID;
        }

        ScanFilter scanFilter = new ScanFilter.Builder()
                .setServiceUuid(new ParcelUuid(serviceUuidParsed))
                .build();

        // Use low-latency scan mode for faster device discovery
        android.bluetooth.le.ScanSettings scanSettings =
                new android.bluetooth.le.ScanSettings.Builder()
                        .setScanMode(android.bluetooth.le.ScanSettings.SCAN_MODE_LOW_LATENCY)
                        .setCallbackType(android.bluetooth.le.ScanSettings.CALLBACK_TYPE_ALL_MATCHES)
                        .build();

        // Clear previous results
        synchronized (sDiscoveredDevices) {
            sDiscoveredDevices.clear();
        }

        try {
            sLeScanner.startScan(
                    java.util.Collections.singletonList(scanFilter),
                    scanSettings,
                    sScanCallback);
            sScanning = true;
            Log.i(TAG, "BLE scan started for MIDI service: " + serviceUuidParsed);
            return true;
        } catch (SecurityException e) {
            Log.e(TAG, "SecurityException starting BLE scan: " + e.getMessage());
            sScanning = false;
            return false;
        }
    }

    /**
     * Stop the current BLE scan if one is running.
     */
    public static void stopScan() {
        if (sLeScanner != null && sScanning) {
            try {
                sLeScanner.stopScan(sScanCallback);
                Log.i(TAG, "BLE scan stopped");
            } catch (SecurityException e) {
                Log.w(TAG, "SecurityException stopping BLE scan: " + e.getMessage());
            }
        }
        sScanning = false;
    }

    /**
     * Check whether a BLE MIDI device is currently connected.
     *
     * Called from C++ via JNI as:
     *   static boolean isConnected()
     *
     * @return true if a BLE MIDI device is connected
     */
    public static boolean isConnected() {
        return sConnected;
    }

    /**
     * Set the connection state. This would be called when a MIDI device
     * connection is established or torn down.
     *
     * @param connected true if a device is connected
     */
    public static void setConnected(boolean connected) {
        sConnected = connected;
    }

    /**
     * Get the list of discovered BLE MIDI devices.
     *
     * Called from C++ via JNI as:
     *   static List getDiscoveredDevices()
     *
     * Returns a List of Maps, where each Map contains:
     *   - "address": String - the Bluetooth MAC address
     *   - "name": String - the device name (or "Unknown BLE MIDI Device")
     *
     * @return List of device info maps; empty list if no devices found
     */
    public static List getDiscoveredDevices() {
        List<Map<String, String>> result = new ArrayList<>();
        synchronized (sDiscoveredDevices) {
            for (Map.Entry<String, String> entry : sDiscoveredDevices.entrySet()) {
                Map<String, String> deviceInfo = new HashMap<>();
                deviceInfo.put("address", entry.getKey());
                deviceInfo.put("name", entry.getValue());
                result.add(deviceInfo);
            }
        }
        Log.d(TAG, "Returning " + result.size() + " discovered BLE MIDI devices");
        return result;
    }

    /**
     * Check whether the app has the required Bluetooth permissions for the
     * current Android version.
     *
     * @param context The context to check permissions against
     * @return true if all required permissions are granted
     */
    private static boolean hasRequiredPermissions(Context context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            // Android 13+ (API 33+)
            if (ContextCompat.checkSelfPermission(context, Manifest.permission.BLUETOOTH_SCAN)
                    != PackageManager.PERMISSION_GRANTED) {
                Log.w(TAG, "BLUETOOTH_SCAN permission not granted");
                return false;
            }
            if (ContextCompat.checkSelfPermission(context, Manifest.permission.BLUETOOTH_CONNECT)
                    != PackageManager.PERMISSION_GRANTED) {
                Log.w(TAG, "BLUETOOTH_CONNECT permission not granted");
                return false;
            }
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            // Android 12 (API 31-32)
            if (ContextCompat.checkSelfPermission(context, Manifest.permission.BLUETOOTH_SCAN)
                    != PackageManager.PERMISSION_GRANTED) {
                Log.w(TAG, "BLUETOOTH_SCAN permission not granted");
                return false;
            }
            if (ContextCompat.checkSelfPermission(context, Manifest.permission.BLUETOOTH_CONNECT)
                    != PackageManager.PERMISSION_GRANTED) {
                Log.w(TAG, "BLUETOOTH_CONNECT permission not granted");
                return false;
            }
        }

        // Location permission is needed for BLE scanning on Android 6-11
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.S) {
            if (ContextCompat.checkSelfPermission(context, Manifest.permission.ACCESS_FINE_LOCATION)
                    != PackageManager.PERMISSION_GRANTED
                    && ContextCompat.checkSelfPermission(context, Manifest.permission.ACCESS_COARSE_LOCATION)
                    != PackageManager.PERMISSION_GRANTED) {
                Log.w(TAG, "Location permission not granted (required for BLE scan on < Android 12)");
                return false;
            }
        }

        return true;
    }

    /**
     * Get the cached context. Used internally by the scan callback for
     * permission checks when accessing device names.
     *
     * @return The cached application context, or null
     */
    private static Context getContext() {
        return sContext;
    }
}
