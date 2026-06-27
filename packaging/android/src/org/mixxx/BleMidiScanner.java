1|package org.mixxx;
2|
3|import android.Manifest;
4|import android.bluetooth.BluetoothAdapter;
5|import android.bluetooth.BluetoothDevice;
6|import android.bluetooth.BluetoothManager;
7|import android.bluetooth.le.ScanCallback;
8|import android.bluetooth.le.ScanFilter;
9|import android.bluetooth.le.ScanResult;
10|import android.bluetooth.le.BluetoothLeScanner;
11|import android.content.Context;
12|import android.content.pm.PackageManager;
13|import android.os.Build;
14|import android.os.ParcelUuid;
15|import android.util.Log;
16|
17|import java.util.ArrayList;
18|import java.util.HashMap;
19|import java.util.List;
20|import java.util.Map;
21|import java.util.UUID;
22|
23|import androidx.core.content.ContextCompat;
24|
25|/**
26| * BLE MIDI device scanner for Mixxx.
27| *
28| * Scans for BLE devices advertising the standard BLE MIDI service UUID
29| * (03B80E5A-EDE8-4B33-A751-6CE34EC4C700) and provides discovered device
30| * information to the C++ layer via JNI.
31| *
32| * This class is instantiated and called from the C++ side through Qt JNI
33| * (BleMidiEnumerator). All public methods invoked from C++ are static.
34| */
35|public class BleMidiScanner {
36|    private static final String TAG = "MixxxBleMidiScanner";
37|
38|    // Standard BLE MIDI Service UUID defined by the MIDI specification
39|    private static final UUID BLE_MIDI_SERVICE_UUID =
40|        UUID.fromString("03B80E5A-EDE8-4B33-A751-6CE34EC4C700");
41|
42|    // Maps BluetoothDevice address -> device name (null if unavailable)
43|    private static final Map<String, String> sDiscoveredDevices = new HashMap<>();
44|
45|    // Tracks whether a scan is currently in progress
46|    private static volatile boolean sScanning = false;
47|
48|    // Tracks whether we have an active BLE connection
49|    private static volatile boolean sConnected = false;
50|
51|    // The BLE scanner handle; null when not scanning
52|    private static BluetoothLeScanner sLeScanner = null;
53|
54|    // Singleton scan callback
55|    private static final ScanCallback sScanCallback = new ScanCallback() {
56|        @Override
57|        public void onScanResult(int callbackType, ScanResult result) {
58|            if (result == null) {
59|                return;
60|            }
61|            BluetoothDevice device = result.getDevice();
62|            String address = device.getAddress();
63|            String name = null;
64|            try {
65|                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
66|                    // Android 13+: name access requires BLUETOOTH_CONNECT (already checked)
67|                    if (ContextCompat.checkSelfPermission(
68|                            getContext(), Manifest.permission.BLUETOOTH_CONNECT)
69|                        == PackageManager.PERMISSION_GRANTED) {
70|                        name = device.getName();
71|                    }
72|                } else {
73|                    name = device.getName();
74|                }
75|            } catch (SecurityException e) {
76|                Log.w(TAG, "Permission denied reading device name for " + address);
77|            }
78|
79|            if (name == null || name.isEmpty()) {
80|                name = "Unknown BLE MIDI Device";
81|            }
82|
83|            synchronized (sDiscoveredDevices) {
84|                if (!sDiscoveredDevices.containsKey(address)) {
85|                    sDiscoveredDevices.put(address, name);
86|                    Log.i(TAG, "Discovered BLE MIDI device: " + name + " [" + address + "]");
87|                }
88|            }
89|        }
90|
91|        @Override
92|        public void onScanFailed(int errorCode) {
93|            Log.e(TAG, "BLE scan failed with error code: " + errorCode);
94|            sScanning = false;
95|        }
96|    };
97|
98|    // Cached context reference (the launching Activity) for permission checks
99|    private static Context sContext;
100|
101|    /**
102|     * Start a BLE scan for devices advertising the MIDI service.
103|     *
104|     * Called from C++ via JNI as:
105|     *   static boolean startScan(Context context, String serviceUuid)
106|     *
107|     * @param context The Android context (typically the Activity)
108|     * @param serviceUuid The service UUID to scan for (e.g. "03B80E5A-EDE8-4B33-A751-6CE34EC4C700")
109|     * @return true if the scan was started successfully, false otherwise
110|     */
111|    public static boolean startScan(Context context, String serviceUuid) {
112|        if (context == null) {
113|            Log.e(TAG, "Cannot start BLE scan: context is null");
114|            return false;
115|        }
116|
117|        sContext = context.getApplicationContext();
118|
119|        // Stop any existing scan before starting a new one
120|        stopScan();
121|
122|        // Verify Bluetooth is available and enabled
123|        BluetoothManager bluetoothManager =
124|            (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
125|        if (bluetoothManager == null) {
126|            Log.e(TAG, "Bluetooth not supported on this device");
127|            return false;
128|        }
129|
130|        BluetoothAdapter bluetoothAdapter = bluetoothManager.getAdapter();
131|        if (bluetoothAdapter == null || !bluetoothAdapter.isEnabled()) {
132|            Log.e(TAG, "Bluetooth is not enabled");
133|            return false;
134|        }
135|
136|        // Check required permissions based on Android version
137|        if (!hasRequiredPermissions(context)) {
138|            Log.e(TAG, "Missing required Bluetooth permissions for BLE scan");
139|            return false;
140|        }
141|
142|        // Get the BLE scanner
143|        sLeScanner = bluetoothAdapter.getBluetoothLeScanner();
144|        if (sLeScanner == null) {
145|            Log.e(TAG, "BluetoothLeScanner not available");
146|            return false;
147|        }
148|
149|        // Build scan filter for the BLE MIDI service
150|        UUID serviceUuidParsed;
151|        try {
152|            serviceUuidParsed = UUID.fromString(serviceUuid);
153|        } catch (IllegalArgumentException e) {
154|            Log.e(TAG, "Invalid service UUID: " + serviceUuid);
155|            // Fall back to standard MIDI UUID
156|            serviceUuidParsed = BLE_MIDI_SERVICE_UUID;
157|        }
158|
159|        ScanFilter scanFilter = new ScanFilter.Builder()
160|                .setServiceUuid(new ParcelUuid(serviceUuidParsed))
161|                .build();
162|
163|        // Use low-latency scan mode for faster device discovery
164|        android.bluetooth.le.ScanSettings scanSettings =
165|                new android.bluetooth.le.ScanSettings.Builder()
166|                        .setScanMode(android.bluetooth.le.ScanSettings.SCAN_MODE_LOW_LATENCY)
167|                        .setCallbackType(android.bluetooth.le.ScanSettings.CALLBACK_TYPE_ALL_MATCHES)
168|                        .build();
169|
170|        // Clear previous results
171|        synchronized (sDiscoveredDevices) {
172|            sDiscoveredDevices.clear();
173|        }
174|
175|        try {
176|            sLeScanner.startScan(
177|                    java.util.Collections.singletonList(scanFilter),
178|                    scanSettings,
179|                    sScanCallback);
180|            sScanning = true;
181|            Log.i(TAG, "BLE scan started for MIDI service: " + serviceUuidParsed);
182|            return true;
183|        } catch (SecurityException e) {
184|            Log.e(TAG, "SecurityException starting BLE scan: " + e.getMessage());
185|            sScanning = false;
186|            return false;
187|        }
188|    }
189|
190|    /**
191|     * Stop the current BLE scan if one is running.
192|     */
193|    public static void stopScan() {
194|        if (sLeScanner != null && sScanning) {
195|            try {
196|                sLeScanner.stopScan(sScanCallback);
197|                Log.i(TAG, "BLE scan stopped");
198|            } catch (SecurityException e) {
199|                Log.w(TAG, "SecurityException stopping BLE scan: " + e.getMessage());
200|            }
201|