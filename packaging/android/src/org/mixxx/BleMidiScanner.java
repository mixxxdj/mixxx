package org.mixxx;

import android.Manifest;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.util.Log;
import androidx.core.content.ContextCompat;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * BLE MIDI Scanner bridge.
 *
 * Delegates to BleMidiController for the actual BLE MIDI implementation.
 * Keeps JNI-compatible method signatures for C++ calls.
 *
 * Matches the kshoji/BLE-MIDI-for-Android architecture:
 * - Scans all BLE devices
 * - Connects via GATT for service discovery
 * - Finds BLE MIDI characteristic
 * - Routes MIDI data to C++ via JNI
 */
public class BleMidiScanner {
    private static final String TAG = "MixxxBleScan";

    private static Context sContext;
    private static boolean sConnected = false;

    /**
     * Start a BLE scan.
     * Called from C++ via JNI as:
     *   static boolean startScan(Context context, String serviceUuid)
     *
     * @param context The Android context
     * @param serviceUuid (Ignored - we scan all devices like kshoji library)
     * @return true if scan started successfully
     */
    public static boolean startScan(Context context, String serviceUuid) {
        sContext = context;
        boolean result = BleMidiController.startScan(context);
        Log.i(TAG, "startScan() -> " + result);
        return result;
    }

    /**
     * Stop scanning.
     * Called from C++ via JNI.
     */
    public static void stopScan() {
        Log.i(TAG, "stopScan()");
        BleMidiController.stopScan();
    }

    /**
     * Get discovered BLE devices.
     * Called from C++ via JNI as:
     *   static List<Map<String, String>> getDiscoveredDevices()
     *
     * @return List of Maps with "address" and "name" keys
     */
    public static List<Map<String, String>> getDiscoveredDevices() {
        List<Map<String, String>> result = new ArrayList<>();
        List<String> devices = BleMidiController.getDiscoveredDevices();
        for (String deviceStr : devices) {
            int colonIdx = deviceStr.indexOf(':');
            if (colonIdx > 0) {
                Map<String, String> deviceInfo = new HashMap<>();
                deviceInfo.put("address", deviceStr.substring(0, colonIdx));
                deviceInfo.put("name", deviceStr.substring(colonIdx + 1));
                result.add(deviceInfo);
            }
        }
        return result;
    }

    /**
     * Check if BLE MIDI device is connected.
     * Called from C++ via JNI as:
     *   static boolean isConnected()
     */
    public static boolean isConnected() {
        return sConnected || BleMidiController.isConnected();
    }

    /**
     * Set connection state.
     * Called from C++ via JNI.
     */
    public static void setConnected(boolean connected) {
        sConnected = connected;
    }

    /**
     * Send MIDI data to a BLE device.
     * Called from C++ via JNI as:
     *   static void writeMidiData(String address, String hexData)
     */
    public static void writeMidiData(String deviceAddress, String hexData) {
        BleMidiController.writeMidiData(deviceAddress, hexData);
    }

    /**
     * Disconnect all BLE MIDI devices.
     */
    public static void disconnectAll() {
        BleMidiController.disconnectAll();
        sConnected = false;
    }

    /**
     * Get a BluetoothDevice by its MAC address.
     *
     * @param address The MAC address to resolve
     * @return The BluetoothDevice, or null
     */
    public static BluetoothDevice getDeviceByAddress(String address) {
        try {
            BluetoothManager bluetoothManager =
                (BluetoothManager) sContext.getSystemService(Context.BLUETOOTH_SERVICE);
            if (bluetoothManager != null && bluetoothManager.getAdapter() != null) {
                return bluetoothManager.getAdapter().getRemoteDevice(address);
            }
        } catch (Exception e) {
            Log.e(TAG, "Failed to get device by address: " + e.getMessage());
        }
        return null;
    }

    /**
     * Check required Bluetooth permissions.
     */
    public static boolean hasRequiredPermissions(Context context) {
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
        return true;
    }
}
