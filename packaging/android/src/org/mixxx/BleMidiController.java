package org.mixxx;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.os.Build;
import android.util.Log;
import java.util.Arrays;
import java.util.List;
import java.util.UUID;

/**
 * BLE MIDI Controller for Mixxx
 *
 * Implements the Apple BLE MIDI protocol based on the BLE-MIDI-for-Android library
 * by K.Shoji (https://github.com/kshoji/BLE-MIDI-for-Android).
 *
 * This handles GATT connection, service discovery, MIDI characteristic notification,
 * and BLE MIDI protocol parsing.
 *
 * Called from C++ via JNI.
 */
public class BleMidiController {
    private static final String TAG = "MixxxBleMidiCtrl";

    // Standard BLE MIDI UUIDs (Apple spec)
    private static final UUID BLE_MIDI_SERVICE_UUID =
        UUID.fromString("03B80E5A-EDE8-4B33-A751-6CE34EC4C700");
    private static final UUID BLE_MIDI_CHARACTERISTIC_UUID =
        UUID.fromString("7772E5DB-3868-4112-A1A9-F2669D106BF3");

    // Client Characteristic Configuration Descriptor UUID
    private static final UUID CCC_DESCRIPTOR_UUID =
        UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");

    private static BluetoothGatt sGatt;
    private static BluetoothGattCharacteristic sMidiCharacteristic;
    private static boolean sConnected = false;
    private static Context sContext;

    // BLE MIDI protocol parsing state
    private static final int MIDI_STATE_TIMESTAMP = 0;
    private static final int MIDI_STATE_WAIT = 1;
    private static final int MIDI_STATE_SIGNAL_2BYTES_2 = 21;
    private static final int MIDI_STATE_SIGNAL_3BYTES_2 = 31;
    private static final int MIDI_STATE_SIGNAL_3BYTES_3 = 32;
    private static final int MIDI_STATE_SIGNAL_SYSEX = 41;
    private static int sMidiState = MIDI_STATE_TIMESTAMP;
    private static int sRunningStatus = 0;

    // JNI callback — called from native code to handle MIDI data
    private static native void nativeOnMidiDataReceived(byte[] data);
    private static native void nativeOnConnectionStateChanged(boolean connected);

    /**
     * Connect to a BLE MIDI device by address.
     *
     * Called from C++ via JNI as:
     *   static boolean connect(Context context, String address, String serviceUuid, String charUuid)
     *
     * @param context The Android context
     * @param deviceAddress The BLE device MAC address
     * @param serviceUuid The BLE MIDI service UUID (ignored, we use the standard one)
     * @param charUuid The BLE MIDI characteristic UUID (ignored, we use the standard one)
     * @return true if connection initiated successfully
     */
    @SuppressLint("MissingPermission")
    public static boolean connect(Context context, String deviceAddress, String serviceUuid, String charUuid) {
        sContext = context.getApplicationContext();

        BluetoothDevice device = BleMidiScanner.getDeviceByAddress(deviceAddress);
        if (device == null) {
            Log.e(TAG, "Device not found in scan results: " + deviceAddress);
            return false;
        }

        Log.i(TAG, "Connecting to BLE MIDI device: " + deviceAddress);
        sGatt = device.connectGatt(sContext, false, sGattCallback);
        return sGatt != null;
    }

    /**
     * Disconnect from the current BLE MIDI device.
     */
    @SuppressLint("MissingPermission")
    public static void disconnect() {
        if (sGatt != null) {
            sGatt.disconnect();
            sGatt.close();
            sGatt = null;
        }
        sConnected = false;
        sMidiCharacteristic = null;
    }

    /**
     * Check if currently connected to a BLE MIDI device.
     */
    public static boolean isConnected() {
        return sConnected && sMidiCharacteristic != null;
    }

    /**
     * Send MIDI data to the connected BLE device.
     * Called from C++ via JNI with hex-encoded data.
     *
     * @param hexData Hex-encoded MIDI bytes to send
     */
    @SuppressLint("MissingPermission")
    public static void writeMidiData(String hexData) {
        if (sGatt == null || sMidiCharacteristic == null || !sConnected) {
            return;
        }

        // Decode hex string to bytes
        byte[] midiData = hexStringToByteArray(hexData);
        if (midiData == null || midiData.length == 0) {
            return;
        }

        // Wrap data in BLE MIDI protocol header
        byte[] packet = wrapBleMidiPacket(midiData);
        if (packet == null) {
            return;
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            sGatt.writeCharacteristic(sMidiCharacteristic, packet,
                BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
        } else {
            sMidiCharacteristic.setValue(packet);
            sMidiCharacteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
            sGatt.writeCharacteristic(sMidiCharacteristic);
        }
    }

    /**
     * Convert hex string to byte array.
     */
    private static byte[] hexStringToByteArray(String hex) {
        if (hex == null || hex.length() % 2 != 0) {
            return null;
        }
        byte[] data = new byte[hex.length() / 2];
        for (int i = 0; i < hex.length(); i += 2) {
            data[i / 2] = (byte) ((Character.digit(hex.charAt(i), 16) << 4)
                + Character.digit(hex.charAt(i + 1), 16));
        }
        return data;
    }

    /**
     * Wrap raw MIDI data in BLE MIDI protocol packet.
     * Format: [timestamp_high(1)] [timestamp_low(1) + status(1)] [data(0-2)] ...
     * Timestamp is 13-bit, incremented at ~1ms intervals.
     */
    private static int sTimestamp = 0;

    private static byte[] wrapBleMidiPacket(byte[] midiData) {
        if (midiData == null || midiData.length == 0) {
            return null;
        }

        sTimestamp = (sTimestamp + 1) & 0x1FFF; // 13-bit counter

        byte tsHigh = (byte) ((sTimestamp >> 7) & 0x01);
        byte tsLow = (byte) (sTimestamp & 0x7F);

        byte[] packet = new byte[1 + midiData.length];
        packet[0] = (byte) (tsHigh | 0x80); // header with timestamp high bit
        packet[1] = (byte) (tsLow | 0x80); // timestamp low + marker

        // Copy MIDI data, adding marker bytes as needed
        int pktIdx = 1;
        for (int i = 0; i < midiData.length; i++) {
            if ((midiData[i] & 0x80) != 0) {
                // Status byte — add timestamp low marker before it
                pktIdx++;
                if (pktIdx >= packet.length) {
                    packet = Arrays.copyOf(packet, packet.length + 1);
                }
                packet[pktIdx] = tsLow;
            }
            pktIdx++;
            if (pktIdx >= packet.length) {
                packet = Arrays.copyOf(packet, packet.length + 1);
            }
            packet[pktIdx] = midiData[i];
        }

        return Arrays.copyOf(packet, pktIdx + 1);
    }

    /**
     * Parse BLE MIDI packet and extract raw MIDI bytes.
     * Handles the BLE MIDI protocol's timestamp headers.
     */
    private static void parseBleMidiPacket(byte[] packet) {
        if (packet == null || packet.length < 2) {
            return;
        }

        int pos = 0;
        while (pos < packet.length) {
            byte b = packet[pos];

            // Check if this is a header byte (bit 7 set)
            if ((b & 0x80) != 0) {
                pos++;
                if (pos >= packet.length)
                    break;
                // Next byte is timestamp low (also has bit 7 set)
                pos++;
                continue;
            }

            // This is a MIDI data byte
            // Parse MIDI message based on running status
            int statusByte = b & 0x7F;
            if ((b & 0x80) != 0) {
                // Status byte
                sRunningStatus = statusByte;
                pos++;
                continue;
            }

            // Data byte — use running status
            int msgType = sRunningStatus & 0xF0;
            int channel = sRunningStatus & 0x0F;

            switch (msgType) {
                case 0x80: // Note Off (3 bytes)
                case 0x90: // Note On (3 bytes)
                case 0xA0: // Poly Pressure (3 bytes)
                case 0xB0: // Control Change (3 bytes)
                case 0xE0: // Pitch Bend (3 bytes)
                    if (pos + 1 < packet.length) {
                        byte[] midiMsg = new byte[] {(byte) sRunningStatus, b, packet[pos + 1]};
                        nativeOnMidiDataReceived(midiMsg);
                        pos += 2;
                    } else {
                        pos++;
                    }
                    break;

                case 0xC0: // Program Change (2 bytes)
                case 0xD0: // Channel Pressure (2 bytes)
                    byte[] midiMsg = new byte[] {(byte) sRunningStatus, b};
                    nativeOnMidiDataReceived(midiMsg);
                    pos++;
                    break;

                case 0xF0: // System message
                    if (sRunningStatus == 0xF0) {
                        // SysEx — collect until 0xF7
                        // For now, just pass through
                        pos++;
                    } else if (sRunningStatus == 0xF1 || sRunningStatus == 0xF3) {
                        // MTC Quarter Frame / Song Select (2 bytes)
                        if (pos + 1 < packet.length) {
                            byte[] sysMsg = new byte[] {(byte) sRunningStatus, packet[pos + 1]};
                            nativeOnMidiDataReceived(sysMsg);
                            pos += 2;
                        } else {
                            pos++;
                        }
                    } else if (sRunningStatus == 0xF2) {
                        // Song Position Pointer (3 bytes)
                        if (pos + 1 < packet.length) {
                            byte[] sysMsg = new byte[] {(byte) sRunningStatus, b, packet[pos + 1]};
                            nativeOnMidiDataReceived(sysMsg);
                            pos += 2;
                        } else {
                            pos++;
                        }
                    } else {
                        // F4-F6, F8-FF are single byte messages
                        byte[] sysMsg = new byte[] {(byte) sRunningStatus};
                        nativeOnMidiDataReceived(sysMsg);
                        pos++;
                    }
                    break;

                default:
                    pos++;
                    break;
            }
        }
    }

    /**
     * GATT callback for BLE MIDI device connection and data transfer.
     */
    @SuppressLint("MissingPermission")
    private static final BluetoothGattCallback sGattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                Log.i(TAG, "GATT connected, discovering services...");
                sGatt.discoverServices();
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                Log.i(TAG, "GATT disconnected");
                sConnected = false;
                sMidiCharacteristic = null;
                nativeOnConnectionStateChanged(false);
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status != BluetoothGatt.GATT_SUCCESS) {
                Log.e(TAG, "Service discovery failed: " + status);
                return;
            }

            // Find BLE MIDI service
            BluetoothGattService midiService = gatt.getService(BLE_MIDI_SERVICE_UUID);
            if (midiService == null) {
                Log.e(TAG, "BLE MIDI service not found on device");
                // List available services for debugging
                for (BluetoothGattService service : gatt.getServices()) {
                    Log.d(TAG, "  Available service: " + service.getUuid());
                }
                return;
            }

            Log.i(TAG, "BLE MIDI service found");

            // Get the MIDI I/O characteristic
            sMidiCharacteristic = midiService.getCharacteristic(BLE_MIDI_CHARACTERISTIC_UUID);
            if (sMidiCharacteristic == null) {
                Log.e(TAG, "BLE MIDI characteristic not found");
                // List available characteristics for debugging
                for (BluetoothGattCharacteristic ch : midiService.getCharacteristics()) {
                    Log.d(TAG, "  Available characteristic: " + ch.getUuid());
                }
                return;
            }

            Log.i(TAG, "BLE MIDI characteristic found, enabling notifications...");

            // Enable notifications on the characteristic
            gatt.setCharacteristicNotification(sMidiCharacteristic, true);

            // Write to the CCC descriptor to enable notifications
            BluetoothGattDescriptor descriptor = sMidiCharacteristic.getDescriptor(CCC_DESCRIPTOR_UUID);
            if (descriptor != null) {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                    gatt.writeDescriptor(descriptor,
                        BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                } else {
                    descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                    gatt.writeDescriptor(descriptor);
                }
            }

            sConnected = true;
            nativeOnConnectionStateChanged(true);
            Log.i(TAG, "BLE MIDI device connected and ready");
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt,
            BluetoothGattCharacteristic characteristic) {
            byte[] data = characteristic.getValue();
            if (data != null && data.length > 0) {
                parseBleMidiPacket(data);
            }
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt,
            BluetoothGattCharacteristic characteristic, byte[] value) {
            if (value != null && value.length > 0) {
                parseBleMidiPacket(value);
            }
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt gatt,
            BluetoothGattDescriptor descriptor, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                Log.i(TAG, "Notifications enabled successfully");
            } else {
                Log.w(TAG, "Failed to enable notifications: " + status);
            }
        }
    };
}
