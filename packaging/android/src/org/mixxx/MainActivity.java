package org.mixxx;

import android.os.Bundle;
import android.text.InputType;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.core.view.WindowInsetsControllerCompat;
import java.lang.reflect.Field;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;
import org.qtproject.qt.android.QtActivityBase;

public class MainActivity extends QtActivityBase {
    private final Set<View> m_noExtractUiPatched = Collections.synchronizedSet(new HashSet<View>());

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Disable drawing over cutout - isn't working
        WindowManager.LayoutParams lp = this.getWindow().getAttributes();
        lp.layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_NEVER;

        // Remove FLAG_FULLSCREEN so the soft keyboard can use adjustResize and
        // stays in inline mode instead of the fullscreen "extract" view that
        // covers the app on landscape. Immersive look is kept via the insets
        // controller below.
        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS);

        // Disable system and navigation bar to prevent accidental back or app switch
        WindowCompat.setDecorFitsSystemWindows(getWindow(), false);
        WindowInsetsControllerCompat windowInsetsController =
            WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView());
        windowInsetsController.setSystemBarsBehavior(
            WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
        windowInsetsController.hide(WindowInsetsCompat.Type.statusBars());
        windowInsetsController.hide(WindowInsetsCompat.Type.navigationBars());

        // The IME opening (adjustResize) can cause the system bars to reappear.
        // Re-hide them on every decor layout pass so the app stays fullscreen.
        getWindow().getDecorView().getViewTreeObserver().addOnGlobalLayoutListener(() -> {
            WindowInsetsControllerCompat c =
                WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView());
            c.hide(WindowInsetsCompat.Type.statusBars());
            c.hide(WindowInsetsCompat.Type.navigationBars());
        });

        enforceInlineIme();
    }

    // Qt never exposes a compact-keyboard toggle: for Qt < 6.8.8 / 6.11.1 /
    // 6.12 it doesn't even set IME_FLAG_NO_EXTRACT_UI, and the IME action it
    // computes (IME_ACTION_DONE by default) makes GBoard render a wide action
    // bar above the keys. Overwriting imeOptions alone is not enough - GBoard
    // keeps its toolbar strip for plain TYPE_CLASS_TEXT inputs regardless.
    // This patch overwrites QtEditText.m_imeOptions with a maximally quiet
    // combination (no action button, no fullscreen extract view, no
    // clipboard/accessory strip) and forces the inputType to the URI text
    // variation, which GBoard renders without any top strip while still
    // allowing full Unicode input. Qt recomputes these options on every
    // keyboard show, so we re-apply the values on each layout pass.
    private void enforceInlineIme() {
        final View decor = getWindow().getDecorView();
        decor.getViewTreeObserver().addOnGlobalFocusChangeListener((oldFocus, newFocus) -> {
            if (newFocus == null || !"org.qtproject.qt.android.QtEditText".equals(newFocus.getClass().getName())) {
                return;
            }
            applyNoExtractUi(newFocus);
        });
    }

    private void applyNoExtractUi(final View editText) {
        if (m_noExtractUiPatched.contains(editText)) {
            return;
        }
        try {
            final Field imeOptions = editText.getClass().getDeclaredField("m_imeOptions");
            imeOptions.setAccessible(true);
            final Field inputType = editText.getClass().getDeclaredField("m_inputType");
            inputType.setAccessible(true);
            setCompactIme(imeOptions, inputType, editText);
            editText.getViewTreeObserver().addOnGlobalLayoutListener(() -> setCompactIme(imeOptions, inputType, editText));
            m_noExtractUiPatched.add(editText);
        } catch (Exception ignored) {
            // Qt version too old/renamed internally; fall back to default behavior.
        }
    }

    private static void setCompactIme(Field imeOptions, Field inputType, View editText) {
        try {
            imeOptions.setInt(editText,
                EditorInfo.IME_ACTION_NONE
                    | EditorInfo.IME_FLAG_NO_EXTRACT_UI
                    | EditorInfo.IME_FLAG_NO_ACCESSORY_ACTION);
        } catch (Exception ignored) {
        }
        try {
            inputType.setInt(editText, InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_URI);
        } catch (Exception ignored) {
        }
    }
}
