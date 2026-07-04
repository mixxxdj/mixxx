import org.mixxx.AndroidScreenManager;
import org.qtproject.qt.android.QtActivityBase;
import android.os.Bundle;
import android.view.WindowManager;

public class MainActivity extends QtActivityBase {
    // ... existing code ...

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // ... existing code ...

        // Apply fullscreen handling for notch devices
        AndroidScreenManager.applyFullScreen(this);
    }