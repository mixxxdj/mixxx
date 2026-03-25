package dev.celtera.libremidi;

import android.media.midi.MidiDevice;
import android.media.midi.MidiManager;
import android.util.Log;

public class MidiDeviceCallback implements MidiManager.OnDeviceOpenedListener {
    private static final String TAG = "libremidi";
    private long nativePtr;
    private boolean isOutput;

    public MidiDeviceCallback(long ptr, boolean output) {
        nativePtr = ptr;
        isOutput = output;
    }
    @Override
    public void onDeviceOpened(MidiDevice device) {
        if (device == null) {
            Log.e(TAG, "Failed to open MIDI device");
            return;
        }

        Log.i(TAG, "MIDI device opened successfully");
        onDeviceOpened(device, nativePtr, isOutput);
    }

    // Native method declaration
    private native void onDeviceOpened(MidiDevice device, long targetPtr, boolean isOutput);

    static {
        // Load the native library containing the JNI implementation
       // System.loadLibrary("remidi");
    }
}
