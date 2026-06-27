package org.mixxx;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.util.Log;
import androidx.core.content.ContextCompat;
import java.util.UUID;

/**
 * BLE MIDI GATT controller for Mixxx.
 *
 * Connects to a BLE MIDI device via BluetoothGatt, discovers the
 * BLE MIDI service and characteristic, enables notifications,
 * and forwards received MIDI data to the C++ layer via JNI.
 *
 * Also supports writing MIDI data back to the device for LED feedback.
 */
public class BleMidiController {
    private static final String TAG = "MixxxBleMidiCtrl";

    // Standard BLE MIDI Service and Characteristic UUIDs
    private static final UUID BLE_MIDI_SERVICE_UUID =
        UUID.fromString("03B80E5A-EDE8-4B33-A751-6CE34EC4C700");
    private static final UUID BLE_MIDI_CHARACTERISTIC_UUID =
        UUID.fromString("7772E5DB-3868-4112-A1C9-F2669D106BF3");

    // Client Characteristic Configuration Descriptor
    private static final UUID CCC_DESCRIPTOR_UUID =
        UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");

    private static BluetoothGatt sGattConnection = null;
    private static BluetoothGattCharacteristic sMidiCharacteristic = null;
    private static String sDeviceAddress = null;
    private static volatile boolean sConnected = false;

    /**
     * Connect to a BLE MIDI device via GATT.
     *
     * Called from C++ via JNI.
     *
     * @param context Android context
     * @param address Bluetooth MAC address of the device
     * @param serviceUuid BLE MIDI service UUID (hex string)
     * @param characteristicUuid BLE MIDI characteristic UUID (hex string)
     * @return true if connection was initiated successfully
     */
    public static boolean connect(Context context, String address,
        String serviceUuid, String characteristicUuid) {
        if (sConnected && sGattConnection != null) {
            Log.i(TAG, "Already connected to a device, disconnect first");
            return false;
        }

        if (!hasBluetoothPermissions(context)) {
            Log.w(TAG, "Missing Bluetooth permissions for GATT connection");
            return false;
        }

        BluetoothManager bluetoothManager =
            (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        if (bluetoothManager == null) {
            Log.e(TAG, "Bluetooth not supported");
            return false;
        }

        BluetoothAdapter adapter = bluetoothManager.getAdapter();
        if (adapter == null || !adapter.isEnabled()) {
            Log.w(TAG, "Bluetooth adapter not available or not enabled");
            return false;
        }

        BluetoothDevice device = adapter.getRemoteDevice(address);
        if (device == null) {
            Log.e(TAG, "Device not found: " + address);
            return false;
        }

        sDeviceAddress = address;
        Log.i(TAG, "Connecting GATT to: " + device.getName() + " [" + address + "]");

        try {
            sGattConnection = device.connectGatt(context, false, sGattCallback);
            return sGattConnection != null;
        } catch (SecurityException e) {
            Log.e(TAG, "SecurityException connecting GATT: " + e.getMessage());
            return false;
        }
    }

    /**
     * Disconnect from the current BLE MIDI device.
     */
    public static void disconnect() {
        if (sGattConnection != null) {
            try {
                sGattConnection.disconnect();
                sGattConnection.close();
            } catch (SecurityException e) {
                Log.w(TAG, "SecurityException disconnecting: " + e.getMessage());
            }
            sGattConnection = null;
            sMidiCharacteristic = null;
            sConnected = false;
            Log.i(TAG, "Disconnected from BLE MIDI device");
        }
    }

    /**
     * Check if currently connected to a BLE MIDI device.
     */
    public static boolean isConnected() {
        return sConnected;
    }

    /**
     * Write MIDI data to the BLE MIDI characteristic.
     *
     * @param hexData Hex-encoded MIDI data (including BLE MIDI timestamp header)
     */
    public static void writeMidiData(String hexData) {
        if (!sConnected || sMidiCharacteristic == null || sGattConnection == null) {
            return;
        }

        try {
            byte[] data = hexStringToByteArray(hexData);
            sMidiCharacteristic.setValue(data);
            sGattConnection.writeCharacteristic(sMidiCharacteristic);
        } catch (SecurityException e) {
            Log.w(TAG, "SecurityException writing MIDI: " + e.getMessage());
        }
    }

    // GATT callback for connection state changes and characteristic notifications
    private static final BluetoothGattCallback sGattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                Log.i(TAG, "GATT connected to " + sDeviceAddress);
                sConnected = true;
                try {
                    gatt.discoverServices();
                } catch (SecurityException e) {
                    Log.e(TAG, "SecurityException discovering services: " + e.getMessage());
                }
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                Log.i(TAG, "GATT disconnected from " + sDeviceAddress);
                sConnected = false;
                sMidiCharacteristic = null;
                try {
                    gatt.close();
                } catch (SecurityException e) {
                    // Ignore
                }
                sGattConnection = null;
                // Notify C++ of disconnection
                nativeOnConnectionStateChanged(false);
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status != BluetoothGatt.GATT_SUCCESS) {
                Log.e(TAG, "Service discovery failed: " + status);
                return;
            }

            Log.i(TAG, "Services discovered, looking for BLE MIDI service");

            BluetoothGattService midiService = gatt.getService(BLE_MIDI_SERVICE_UUID);
            if (midiService == null) {
                Log.w(TAG, "BLE MIDI service not found on device");
                // Log all services for debugging
                for (BluetoothGattService service : gatt.getServices()) {
                    Log.d(TAG, "Found service: " + service.getUuid());
                }
                return;
            }

            sMidiCharacteristic = midiService.getCharacteristic(BLE_MIDI_CHARACTERISTIC_UUID);
            if (sMidiCharacteristic == null) {
                Log.w(TAG, "BLE MIDI characteristic not found in service");
                return;
            }

            Log.i(TAG, "BLE MIDI characteristic found, enabling notifications");

            try {
                gatt.setCharacteristicNotification(sMidiCharacteristic, true);

                // Enable notifications via CCC descriptor
                BluetoothGattDescriptor descriptor =
                    sMidiCharacteristic.getDescriptor(CCC_DESCRIPTOR_UUID);
                if (descriptor != null) {
                    descriptor.setValue(
                        BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                    gatt.writeDescriptor(descriptor);
                } else {
                    Log.w(TAG, "CCC descriptor not found");
                }

                // Notify C++ of successful connection
                nativeOnConnectionStateChanged(true);
            } catch (SecurityException e) {
                Log.e(TAG, "SecurityException enabling notifications: " + e.getMessage());
            }
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt,
            BluetoothGattCharacteristic characteristic) {
            if (BLE_MIDI_CHARACTERISTIC_UUID.equals(characteristic.getUuid())) {
                byte[] data = characteristic.getValue();
                if (data != null && data.length > 0) {
                    Log.d(TAG, "MIDI data received: " + data.length + " bytes");
                    nativeOnMidiDataReceived(data);
                }
            }
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt gatt,
            BluetoothGattDescriptor descriptor, int status) {
            if (CCC_DESCRIPTOR_UUID.equals(descriptor.getUuid())) {
                if (status == BluetoothGatt.GATT_SUCCESS) {
                    Log.i(TAG, "BLE MIDI notifications enabled successfully");
                } else {
                    Log.w(TAG, "Failed to enable BLE MIDI notifications: " + status);
                }
            }
        }
    };

    // JNI callbacks to C++
    private static native void nativeOnMidiDataReceived(byte[] data);
    private static native void nativeOnConnectionStateChanged(boolean connected);

    // Utility: convert hex string to byte array
    private static byte[] hexStringToByteArray(String s) {
        int len = s.length();
        byte[] data = new byte[len / 2];
        for (int i = 0; i < len; i += 2) {
            data[i / 2] = (byte) ((Character.digit(s.charAt(i), 16) << 4)
                + Character.digit(s.charAt(i + 1), 16));
        }
        return data;
    }

    private static boolean hasBluetoothPermissions(Context context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            return ContextCompat.checkSelfPermission(context, Manifest.permission.BLUETOOTH_CONNECT)
                == PackageManager.PERMISSION_GRANTED;
        }
        return true;
    }
}
