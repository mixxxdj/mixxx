package com.libremidi.example;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;
import android.widget.Button;
import android.widget.ScrollView;
import android.view.ViewGroup.LayoutParams;
import android.widget.LinearLayout;
import android.graphics.Typeface;
import android.util.TypedValue;

public class MainActivity extends AppCompatActivity {
    
    static {
        System.loadLibrary("libremidi-example");
    }
    
    private TextView deviceListTextView;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setPadding(16, 16, 16, 16);
        
        TextView titleView = new TextView(this);
        titleView.setText("LibreMIDI Android Example");
        titleView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 24);
        titleView.setTypeface(null, Typeface.BOLD);
        titleView.setPadding(0, 0, 0, 16);
        layout.addView(titleView);
        
        TextView versionView = new TextView(this);
        versionView.setText(getLibremidiVersion());
        versionView.setPadding(0, 0, 0, 16);
        layout.addView(versionView);
        
        Button refreshButton = new Button(this);
        refreshButton.setText("Refresh MIDI Devices");
        refreshButton.setOnClickListener(v -> refreshDevices());
        layout.addView(refreshButton);
        
        ScrollView scrollView = new ScrollView(this);
        deviceListTextView = new TextView(this);
        deviceListTextView.setTypeface(Typeface.MONOSPACE);
        deviceListTextView.setPadding(0, 16, 0, 0);
        scrollView.addView(deviceListTextView);
        
        LinearLayout.LayoutParams scrollParams = new LinearLayout.LayoutParams(
            LayoutParams.MATCH_PARENT, 
            LayoutParams.MATCH_PARENT
        );
        scrollParams.weight = 1;
        layout.addView(scrollView, scrollParams);
        
        setContentView(layout);
        
        refreshDevices();
    }
    
    private void refreshDevices() {
        String devices = getMidiDevices();
        deviceListTextView.setText(devices);
    }
    
    public native String getMidiDevices();
    public native String getLibremidiVersion();
}