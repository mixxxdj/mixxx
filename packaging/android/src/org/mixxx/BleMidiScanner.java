package org.mixxx;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.ParcelUuid;
import android.util.Log;
import androidx.core.content.ContextCompat;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

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

    // Cached context reference for permission checks in scan callback
    private static Context sContext;

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

            // Fallback to name in scan record if device.getName() returns null
            if (name == null || name.isEmpty()) {
                if (result.getScanRecord() != null) {
                    name = result.getScanRecord().getDeviceName();
                }
            }

            if (name == null) {
                name = "";
            }

            // Check if device matches name or UUID
            boolean isMidi = false;
            String lowerName = name.toLowerCase();
            if (lowerName.contains("ddj-") || lowerName.contains("pioneer") || lowerName.contains("flx")) {
                isMidi = true;
            }

            if (!isMidi && result.getScanRecord() != null) {
                java.util.List<ParcelUuid> uuids = result.getScanRecord().getServiceUuids();
                if (uuids != null) {
                    for (ParcelUuid uuid : uuids) {
                        if (uuid.getUuid().equals(BLE_MIDI_SERVICE_UUID)) {
                            isMidi = true;
                            break;
                        }
                    }
                }
            }

            // If it's not a MIDI/Mixxx-compatible device, skip it!
            if (!isMidi) {
                return;
            }

            if (name.isEmpty()) {
                name = "BLE MIDI Device";
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

    /**
     * Start a BLE scan for devices advertising the MIDI service.
     * Also scans for Pioneer DDJ controllers by name pattern.
     *
     * Called from C++ via JNI.
     *
     * @param context The Android context (typically the Activity)
     * @param serviceUuid The service UUID to scan for (e.g. BLE MIDI service)
     * @return true if the scan was started successfully
     */
    public static boolean startScan(Context context, String serviceUuid) {
        sContext = context.getApplicationContext();

        if (!hasRequiredPermissions(context)) {
            Log.w(TAG, "Missing required Bluetooth permissions for BLE scan");
            return false;
        }

        // Verify Bluetooth is available and enabled
        BluetoothManager bluetoothManager =
            (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        if (bluetoothManager == null) {
            Log.e(TAG, "Bluetooth not supported on this device");
            return false;
        }

        BluetoothAdapter bluetoothAdapter = bluetoothManager.getAdapter();
        if (bluetoothAdapter == null || !bluetoothAdapter.isEnabled()) {
            Log.w(TAG, "Bluetooth adapter not available or not enabled");
            return false;
        }

        sLeScanner = bluetoothAdapter.getBluetoothLeScanner();
        if (sLeScanner == null) {
            Log.e(TAG, "BLE scanner not available");
            return false;
        }

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
            // Log scan start with detailed info
            Log.i(TAG,
                "BLE scan starting - Bluetooth enabled: "
                    + bluetoothAdapter.isEnabled()
                    + ", location enabled: "
                    + isLocationEnabled(context));

            // Perform an unfiltered scan (null filters) for maximum robustness across devices
            sLeScanner.startScan(
                null,
                scanSettings,
                sScanCallback);
            sScanning = true;
            Log.i(TAG, "BLE scan started unfiltered (manual scan matching enabled)");
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
     * Set the connection state.
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
     * @return List of device info maps with "address" and "name" keys
     */
    public static List<Map<String, String>> getDiscoveredDevices() {
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
     * Check whether the app has the required Bluetooth permissions.
     *
     * @param context The context to check permissions against
     * @return true if all required permissions are granted
     */
    private static boolean hasRequiredPermissions(Context context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
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
                && ContextCompat.checkSelfPermission(
                       context, Manifest.permission.ACCESS_COARSE_LOCATION)
                    != PackageManager.PERMISSION_GRANTED) {
                Log.w(TAG, "Location permission not granted (required for BLE scan on < Android 12)");
                return false;
            }
        }

        return true;
    }

    @SuppressWarnings("deprecation")
    private static boolean isLocationEnabled(Context context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            android.location.LocationManager lm =
                (android.location.LocationManager)
                    context.getSystemService(Context.LOCATION_SERVICE);
            return lm != null && lm.isLocationEnabled();
        }
        // On older versions, check Settings.Secure
        try {
            return android.provider.Settings.Secure.getString(
                                                       context.getContentResolver(),
                                                       android.provider.Settings.Secure.LOCATION_MODE)
                       .equals("0")
                == false;
        } catch (Exception e) {
            return true; // Assume enabled if we can't check
        }
    }

    private static Context getContext() {
        return sContext;
    }
}
