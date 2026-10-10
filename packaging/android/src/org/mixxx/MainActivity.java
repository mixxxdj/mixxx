package org.mixxx;

import android.os.Bundle;
import android.text.InputType;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.core.view.WindowInsetsControllerCompat;
import java.lang.reflect.Field;
import org.qtproject.qt.android.QtActivityBase;

public class MainActivity extends QtActivityBase {
    private static final String TAG = "MixxxMainActivity";
    private static final String QT_EDIT_TEXT = "org.qtproject.qt.android.QtEditText";

    private int m_lastLoggedInputType = Integer.MIN_VALUE;
    private int m_lastLoggedImeOptions = Integer.MIN_VALUE;

    @Override
    @SuppressWarnings("deprecation") // Qt sets FLAG_FULLSCREEN/FLAG_LAYOUT_NO_LIMITS;
                                     // clearing them is still required for adjustResize.
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Never draw into the display cutout.
        WindowManager.LayoutParams lp = this.getWindow().getAttributes();
        lp.layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_NEVER;

        // Keep the soft keyboard inline (adjustResize) instead of the fullscreen
        // extract view, while the app stays edge-to-edge.
        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS);
        WindowCompat.setDecorFitsSystemWindows(getWindow(), false);

        // Hide the system/navigation bars to prevent accidental back or app switch.
        final View decor = getWindow().getDecorView();
        WindowInsetsControllerCompat controller =
            WindowCompat.getInsetsController(getWindow(), decor);
        controller.setSystemBarsBehavior(
            WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
        controller.hide(WindowInsetsCompat.Type.statusBars());
        controller.hide(WindowInsetsCompat.Type.navigationBars());

        // The IME can make the system bars reappear; re-hide them on every layout pass.
        decor.getViewTreeObserver().addOnGlobalLayoutListener(() -> {
            WindowInsetsControllerCompat c =
                WindowCompat.getInsetsController(getWindow(), decor);
            c.hide(WindowInsetsCompat.Type.statusBars());
            c.hide(WindowInsetsCompat.Type.navigationBars());
        });

        installImeOptionsHook();
    }

    // Qt derives the IME inputType/imeOptions from the QML inputMethodHints
    // (e.g. Qt.ImhUrlCharactersOnly -> TYPE_TEXT_VARIATION_URI). We log what Qt
    // chose and, for URI inputs with an explicit action key, strip the extract UI
    // and the accessory strip while keeping the action.
    private void installImeOptionsHook() {
        final View decor = getWindow().getDecorView();
        decor.getViewTreeObserver().addOnGlobalFocusChangeListener((oldFocus, newFocus) -> {
            if (isQtEditText(newFocus)) {
                handleImeOptions(newFocus);
            }
        });
        decor.getViewTreeObserver().addOnGlobalLayoutListener(() -> {
            View focused = decor.findFocus();
            if (isQtEditText(focused)) {
                handleImeOptions(focused);
            }
        });
    }

    private static boolean isQtEditText(View view) {
        return view != null && QT_EDIT_TEXT.equals(view.getClass().getName());
    }

    private void handleImeOptions(View editText) {
        try {
            int inputType = readIntField(editText, "m_inputType");
            int imeOptions = readIntField(editText, "m_imeOptions");

            if (inputType != m_lastLoggedInputType || imeOptions != m_lastLoggedImeOptions) {
                m_lastLoggedInputType = inputType;
                m_lastLoggedImeOptions = imeOptions;
                Log.i(TAG, "QtEditText inputType=" + describeInputType(inputType) + ", imeOptions=" + describeImeOptions(imeOptions) + ", URI variation=" + hasUriVariation(inputType));
            }

            if (!hasUriVariation(inputType) || !hasPatchableAction(imeOptions)) {
                return;
            }
            int patched = imeOptions | EditorInfo.IME_FLAG_NO_EXTRACT_UI
                | EditorInfo.IME_FLAG_NO_ACCESSORY_ACTION;
            if (patched == imeOptions) {
                return;
            }
            writeIntField(editText, "m_imeOptions", patched);
            Log.i(TAG, "Patched imeOptions " + describeImeOptions(imeOptions) + " -> " + describeImeOptions(patched));
        } catch (Exception e) {
            Log.w(TAG, "Unable to access QtEditText IME state (Qt internals changed?)", e);
        }
    }

    private static boolean hasUriVariation(int inputType) {
        return (inputType & InputType.TYPE_MASK_VARIATION) == InputType.TYPE_TEXT_VARIATION_URI;
    }

    // Done/Send/Search/Next/Previous only; EnterKeyDefault/Go (both IME_ACTION_GO)
    // and EnterKeyReturn (no action) are left as Qt produced them.
    private static boolean hasPatchableAction(int imeOptions) {
        switch (imeOptions & EditorInfo.IME_MASK_ACTION) {
            case EditorInfo.IME_ACTION_DONE:
            case EditorInfo.IME_ACTION_SEND:
            case EditorInfo.IME_ACTION_SEARCH:
            case EditorInfo.IME_ACTION_NEXT:
            case EditorInfo.IME_ACTION_PREVIOUS:
                return true;
            default:
                return false;
        }
    }

    private static int readIntField(Object target, String name) throws Exception {
        Field field = target.getClass().getDeclaredField(name);
        field.setAccessible(true);
        return field.getInt(target);
    }

    private static void writeIntField(Object target, String name, int value) throws Exception {
        Field field = target.getClass().getDeclaredField(name);
        field.setAccessible(true);
        field.setInt(target, value);
    }

    private static String describeInputType(int inputType) {
        String inputClass;
        switch (inputType & InputType.TYPE_MASK_CLASS) {
            case InputType.TYPE_CLASS_TEXT:
                inputClass = "TEXT";
                break;
            case InputType.TYPE_CLASS_NUMBER:
                inputClass = "NUMBER";
                break;
            case InputType.TYPE_CLASS_PHONE:
                inputClass = "PHONE";
                break;
            case InputType.TYPE_CLASS_DATETIME:
                inputClass = "DATETIME";
                break;
            default:
                inputClass = "0x" + Integer.toHexString(inputType & InputType.TYPE_MASK_CLASS);
        }

        String variation;
        switch (inputType & InputType.TYPE_MASK_VARIATION) {
            case InputType.TYPE_TEXT_VARIATION_URI:
                variation = "URI";
                break;
            case InputType.TYPE_TEXT_VARIATION_EMAIL_ADDRESS:
                variation = "EMAIL";
                break;
            case InputType.TYPE_TEXT_VARIATION_PASSWORD:
                variation = "PASSWORD";
                break;
            case InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD:
                variation = "VISIBLE_PASSWORD";
                break;
            default:
                variation = "0x" + Integer.toHexString(inputType & InputType.TYPE_MASK_VARIATION);
        }

        return "0x" + Integer.toHexString(inputType)
            + " (class=" + inputClass + ", variation=" + variation + ")";
    }

    private static String describeImeOptions(int imeOptions) {
        StringBuilder result = new StringBuilder("0x" + Integer.toHexString(imeOptions));
        result.append(" action=").append(describeAction(imeOptions & EditorInfo.IME_MASK_ACTION));
        if ((imeOptions & EditorInfo.IME_FLAG_NO_EXTRACT_UI) != 0) {
            result.append(" NO_EXTRACT_UI");
        }
        if ((imeOptions & EditorInfo.IME_FLAG_NO_ACCESSORY_ACTION) != 0) {
            result.append(" NO_ACCESSORY_ACTION");
        }
        if ((imeOptions & EditorInfo.IME_FLAG_NO_ENTER_ACTION) != 0) {
            result.append(" NO_ENTER_ACTION");
        }
        return result.toString();
    }

    private static String describeAction(int action) {
        switch (action) {
            case EditorInfo.IME_ACTION_UNSPECIFIED:
                return "UNSPECIFIED";
            case EditorInfo.IME_ACTION_NONE:
                return "NONE";
            case EditorInfo.IME_ACTION_GO:
                return "GO";
            case EditorInfo.IME_ACTION_SEARCH:
                return "SEARCH";
            case EditorInfo.IME_ACTION_SEND:
                return "SEND";
            case EditorInfo.IME_ACTION_NEXT:
                return "NEXT";
            case EditorInfo.IME_ACTION_DONE:
                return "DONE";
            case EditorInfo.IME_ACTION_PREVIOUS:
                return "PREVIOUS";
            default:
                return "0x" + Integer.toHexString(action);
        }
    }
}
