package org.mixxx;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.UUID;

/**
 * BLE MIDI scanner and controller for Android.
 *
 * Based on kshoji/BLE-MIDI-for-Android library architecture:
 * - Scans all BLE devices
 * - Connects to each via GATT
 * - Discovers services and finds BLE MIDI service
 * - Enables notifications on MIDI characteristic
 * - Parses BLE MIDI protocol (Apple spec)
 *
 * Called from C++ via JNI.
 */
@SuppressWarnings("deprecation")
public class BleMidiController {
    private static final String TAG = "MixxxBleMidi";

    // BLE MIDI UUIDs (Apple spec)
    private static final UUID BLE_MIDI_SERVICE_UUID =
        UUID.fromString("03B80E5A-EDE8-4B33-A751-6CE34EC4C700");
    private static final UUID BLE_MIDI_CHAR_UUID =
        UUID.fromString("7772E5DB-3868-4112-A1A9-F2669D106BF3");
    private static final UUID CCC_DESCRIPTOR_UUID =
        UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");

    private static Context sContext;
    private static BluetoothAdapter sBluetoothAdapter;
    private static BluetoothLeScanner sLeScanner;
    private static boolean sScanning = false;
    private static boolean sConnected = false;

    // Store discovered device info: address -> name
    private static final Map<String, String> sDiscoveredDevices = new HashMap<>();
    // Connected GATT instances: address -> Gatt
    private static final Map<String, BluetoothGatt> sConnectedGatts = new HashMap<>();
    // MIDI characteristics: address -> characteristic
    private static final Map<String, BluetoothGattCharacteristic> sMidiChars = new HashMap<>();

    // JNI callbacks
    private static native void nativeOnMidiDataReceived(String deviceAddress, byte[] data);
    private static native void nativeOnDeviceConnected(String deviceAddress, String deviceName);
    private static native void nativeOnDeviceDisconnected(String deviceAddress);

    /**
     * Initialize the BLE MIDI controller.
     * Called from C++ during initialization.
     */
    public static void initialize(Context context) {
        sContext = context.getApplicationContext();

        BluetoothManager bm = (BluetoothManager) sContext
                                  .getSystemService(Context.BLUETOOTH_SERVICE);
        if (bm == null) {
            Log.e(TAG, "Bluetooth not supported");
            return;
        }
        sBluetoothAdapter = bm.getAdapter();
        if (sBluetoothAdapter == null || !sBluetoothAdapter.isEnabled()) {
            Log.w(TAG, "Bluetooth not available or disabled");
            return;
        }
        Log.i(TAG, "BLE MIDI initialized");
    }

    /**
     * Start scanning for BLE MIDI devices.
     * Like kshoji library: connects to each found device via GATT and checks for MIDI service.
     */
    @SuppressLint("MissingPermission")
    public static boolean startScan(Context context) {
        if (sContext == null) {
            initialize(context);
        }
        if (sBluetoothAdapter == null || !sBluetoothAdapter.isEnabled()) {
            Log.w(TAG, "Bluetooth not available, cannot scan");
            return false;
        }

        // Log Bluetooth state for debugging
        Log.i(TAG, "Starting BLE scan - BT enabled: " + sBluetoothAdapter.isEnabled());

        sLeScanner = sBluetoothAdapter.getBluetoothLeScanner();
        if (sLeScanner == null) {
            Log.e(TAG, "BLE scanner not available");
            return false;
        }

        // Clear previous results
        sDiscoveredDevices.clear();
        sScanning = false;

        ScanSettings settings = new ScanSettings.Builder()
                                    .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
                                    .setCallbackType(ScanSettings.CALLBACK_TYPE_ALL_MATCHES)
                                    .build();

        try {
            // No filters - scan all devices (like kshoji library)
            sLeScanner.startScan(null, settings, sScanCallback);
            sScanning = true;
            Log.i(TAG, "BLE scan started (unfiltered)");
            return true;
        } catch (SecurityException e) {
            Log.e(TAG, "SecurityException starting scan: " + e.getMessage());
            return false;
        }
    }

    /**
     * Stop scanning.
     */
    @SuppressLint("MissingPermission")
    public static void stopScan() {
        if (sLeScanner != null && sScanning) {
            try {
                sLeScanner.stopScan(sScanCallback);
                Log.i(TAG, "BLE scan stopped");
            } catch (SecurityException e) {
                Log.w(TAG, "SecurityException stopping scan: " + e.getMessage());
            }
        }
        sScanning = false;
    }

    /**
     * Get discovered devices as a list of "address:name" strings.
     */
    public static List<String> getDiscoveredDevices() {
        List<String> result = new ArrayList<>();
        synchronized (sDiscoveredDevices) {
            for (Map.Entry<String, String> entry : sDiscoveredDevices.entrySet()) {
                result.add(entry.getKey() + ":" + entry.getValue());
            }
        }
        return result;
    }

    /**
     * Check if connected to a BLE MIDI device.
     */
    public static boolean isConnected() {
        return sConnected && !sConnectedGatts.isEmpty();
    }

    /**
     * Disconnect from all BLE MIDI devices.
     */
    @SuppressLint("MissingPermission")
    public static void disconnectAll() {
        for (BluetoothGatt gatt : sConnectedGatts.values()) {
            if (gatt != null) {
                gatt.disconnect();
                gatt.close();
            }
        }
        sConnectedGatts.clear();
        sMidiChars.clear();
        sConnected = false;
    }

    /**
     * Send MIDI data to a connected BLE device.
     * Called from C++ via JNI with hex-encoded data.
     *
     * @param deviceAddress The MAC address of the target device
     * @param hexData Hex-encoded MIDI bytes
     */
    @SuppressLint("MissingPermission")
    public static void writeMidiData(String deviceAddress, String hexData) {
        BluetoothGattCharacteristic ch = sMidiChars.get(deviceAddress);
        BluetoothGatt gatt = sConnectedGatts.get(deviceAddress);
        if (ch == null || gatt == null) {
            return;
        }

        byte[] midiData = hexStringToByteArray(hexData);
        if (midiData == null || midiData.length == 0) {
            return;
        }

        // Wrap in BLE MIDI protocol: [timestamp header][MIDI data]
        byte[] packet = wrapBleMidiPacket(midiData);
        if (packet == null)
            return;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            gatt.writeCharacteristic(ch, packet,
                BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
        } else {
            ch.setValue(packet);
            ch.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
            gatt.writeCharacteristic(ch);
        }
    }

    // ---- BLE MIDI Protocol helpers ----

    private static int sTimestamp = 0;

    private static byte[] wrapBleMidiPacket(byte[] midiData) {
        if (midiData == null || midiData.length == 0)
            return null;

        sTimestamp = (sTimestamp + 1) & 0x1FFF;

        // BLE MIDI packet format:
        // [timestamp_high 1 byte (bit7=1)] [timestamp_low 1 byte (bit7=1)] [status byte (bit7=1)] [data bytes (bit7=0)]...
        byte[] packet = new byte[midiData.length + 2];
        packet[0] = (byte) (0x80 | ((sTimestamp >> 7) & 0x3F));
        packet[1] = (byte) (0x80 | (sTimestamp & 0x7F));
        System.arraycopy(midiData, 0, packet, 2, midiData.length);
        return packet;
    }

    private static byte[] hexStringToByteArray(String hex) {
        if (hex == null || hex.length() % 2 != 0)
            return null;
        byte[] data = new byte[hex.length() / 2];
        for (int i = 0; i < hex.length(); i += 2) {
            data[i / 2] = (byte) ((Character.digit(hex.charAt(i), 16) << 4)
                + Character.digit(hex.charAt(i + 1), 16));
        }
        return data;
    }

    // ---- Scan Callback (like kshoji library's ScanCallback) ----

    private static final ScanCallback sScanCallback = new ScanCallback() {
        @Override
        @SuppressLint("MissingPermission")
        public void onScanResult(int callbackType, ScanResult result) {
            if (result == null || result.getDevice() == null)
                return;

            BluetoothDevice device = result.getDevice();
            String address = device.getAddress();

            // Only connect to LE or DUAL mode devices (like kshoji library)
            int type = device.getType();
            if (type != BluetoothDevice.DEVICE_TYPE_LE
                && type != BluetoothDevice.DEVICE_TYPE_DUAL) {
                return;
            }

            // Skip if already connected
            if (sConnectedGatts.containsKey(address))
                return;

            Log.d(TAG, "Found BLE device: " + address);

            // Store device info
            String name = device.getName();
            if (name == null || name.isEmpty()) {
                if (result.getScanRecord() != null) {
                    name = result.getScanRecord().getDeviceName();
                }
            }
            if (name == null)
                name = "Unknown";
            synchronized (sDiscoveredDevices) {
                sDiscoveredDevices.put(address, name);
            }

            // Like kshoji library: connect to EACH device via GATT
            try {
                device.connectGatt(sContext, false, sGattCallback);
                Log.d(TAG, "Connecting GATT to " + address + " (" + name + ")");
            } catch (SecurityException e) {
                Log.w(TAG, "SecurityException connecting to " + address + ": " + e.getMessage());
            }
        }

        @Override
        public void onScanFailed(int errorCode) {
            Log.e(TAG, "BLE scan failed with error code: " + errorCode);
            sScanning = false;
        }
    };

    // ---- GATT Callback (like kshoji library's BleMidiCallback) ----

    private static final BluetoothGattCallback sGattCallback = new BluetoothGattCallback() {
        @Override
        @SuppressLint("MissingPermission")
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            if (gatt.getDevice() == null)
                return;
            String addr = gatt.getDevice().getAddress();

            if (newState == BluetoothProfile.STATE_CONNECTED) {
                Log.i(TAG, "GATT connected: " + addr);
                // Start service discovery immediately
                gatt.discoverServices();
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                Log.i(TAG, "GATT disconnected: " + addr);
                sConnectedGatts.remove(addr);
                sMidiChars.remove(addr);
                if (sConnectedGatts.isEmpty()) {
                    sConnected = false;
                }
                try {
                    gatt.close();
                } catch (Exception e) {
                    // Ignore
                }
                nativeOnDeviceDisconnected(addr);
            }
        }

        @Override
        @SuppressLint("MissingPermission")
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (gatt.getDevice() == null)
                return;
            String addr = gatt.getDevice().getAddress();

            if (status != BluetoothGatt.GATT_SUCCESS) {
                Log.w(TAG, "Service discovery failed for " + addr + ": " + status);
                return;
            }

            // Look for BLE MIDI service
            BluetoothGattService midiService = gatt.getService(BLE_MIDI_SERVICE_UUID);
            if (midiService == null) {
                // Not a MIDI device - disconnect
                Log.d(TAG, addr + " has no BLE MIDI service, disconnecting");
                gatt.disconnect();
                return;
            }

            Log.i(TAG, "Found BLE MIDI service on " + addr);

            // Get MIDI characteristic
            BluetoothGattCharacteristic midiChar =
                midiService.getCharacteristic(BLE_MIDI_CHAR_UUID);
            if (midiChar == null) {
                Log.w(TAG, "No MIDI characteristic on " + addr + ", disconnecting");
                gatt.disconnect();
                return;
            }

            Log.i(TAG, "Found BLE MIDI characteristic on " + addr);

            // Register for notifications
            gatt.setCharacteristicNotification(midiChar, true);

            // Enable CCC descriptor
            BluetoothGattDescriptor ccc = midiChar.getDescriptor(CCC_DESCRIPTOR_UUID);
            if (ccc != null) {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                    gatt.writeDescriptor(ccc, BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                } else {
                    ccc.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                    gatt.writeDescriptor(ccc);
                }
            }

            // Store for later use
            sConnectedGatts.put(addr, gatt);
            sMidiChars.put(addr, midiChar);
            sConnected = true;

            String deviceName = gatt.getDevice().getName();
            if (deviceName == null)
                deviceName = addr;

            Log.i(TAG, "BLE MIDI device ready: " + deviceName + " (" + addr + ")");
            nativeOnDeviceConnected(addr, deviceName);
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt,
            BluetoothGattCharacteristic characteristic) {
            if (gatt.getDevice() == null)
                return;
            String addr = gatt.getDevice().getAddress();
            byte[] value = characteristic.getValue();
            if (value != null && value.length > 0) {
                // Parse BLE MIDI protocol - strip timestamp headers
                byte[] midiData = parseBleMidiPacket(value);
                if (midiData != null && midiData.length > 0) {
                    nativeOnMidiDataReceived(addr, midiData);
                }
            }
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt,
            BluetoothGattCharacteristic characteristic, byte[] value) {
            if (gatt.getDevice() == null)
                return;
            String addr = gatt.getDevice().getAddress();
            if (value != null && value.length > 0) {
                byte[] midiData = parseBleMidiPacket(value);
                if (midiData != null && midiData.length > 0) {
                    nativeOnMidiDataReceived(addr, midiData);
                }
            }
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt gatt,
            BluetoothGattDescriptor descriptor, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                Log.i(TAG, "CCC descriptor written successfully");
            } else {
                Log.w(TAG, "CCC descriptor write failed: " + status);
            }
        }
    };

    /**
     * Parse BLE MIDI packet - strip timestamp headers and extract raw MIDI bytes.
     * BLE MIDI format: [timestamp1(bit7=1)] [timestamp2(bit7=1)] [midi_status(bit7=1)] [midi_data(bit7=0)]...
     */
    private static byte[] parseBleMidiPacket(byte[] packet) {
        if (packet == null || packet.length < 2)
            return null;

        // Skip timestamp header bytes (bit7 set)
        int pos = 0;
        while (pos < packet.length && (packet[pos] & 0x80) != 0) {
            pos++;
        }

        if (pos >= packet.length)
            return null;

        // Collect raw MIDI bytes
        byte[] result = new byte[packet.length - pos];
        System.arraycopy(packet, pos, result, 0, result.length);
        return result;
    }

    /**
     * Prepare for garbage collection - disconnect all.
     */
    public static void cleanup() {
        stopScan();
        disconnectAll();
    }
}
