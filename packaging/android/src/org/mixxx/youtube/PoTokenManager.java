package org.mixxx.youtube;

import android.content.Context;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

/**
 * Singleton owner of the PO-token WebView. Caches one generator per process;
 * recreates it when the integrity token expires. Called from C++ (Qt main
 * thread) via JNI — blocking calls run the init/token fetch on a worker and
 * wait with timeouts so the UI thread never deadlocks.
 */
public class PoTokenManager {
    private static final String TAG = "PoTokenManager";
    private static final long INIT_TIMEOUT_SEC = 60;
    private static final long TOKEN_TIMEOUT_SEC = 45;

    private static PoTokenManager sInstance;
    private Context mAppContext;
    private PoTokenWebView mGenerator;
    private final Object mLock = new Object();

    private PoTokenManager() {
    }

    private volatile String mLastError = "never attempted";

    public static synchronized PoTokenManager getInstance() {
        if (sInstance == null) {
            sInstance = new PoTokenManager();
        }
        return sInstance;
    }

    public void init(Context context) {
        mAppContext = context.getApplicationContext();
        mLastError = "init ok, generator not yet created";
    }

    /** Last failure reason, for surfacing Java-side errors to C++ logs. */
    public String getLastError() {
        return mLastError;
    }

    private void setError(String e) {
        mLastError = e;
        Log.e(TAG, e);
    }

    private boolean ensureGenerator() {
        synchronized (mLock) {
            if (mGenerator != null && mGenerator.isReady()) {
                return true;
            }
            mGenerator = null;
        }
        if (mAppContext == null) {
            setError("ensureGenerator: no context (MainActivity.init not called?)");
            return false;
        }
        final CountDownLatch latch = new CountDownLatch(1);
        final boolean[] ok = new boolean[1];
        final PoTokenWebView[] gen = new PoTokenWebView[1];
        // create() posts WebView work to the main thread itself.
        PoTokenWebView.create(mAppContext, new PoTokenWebView.InitCallback() {
            @Override
            public void onReady(PoTokenWebView g) {
                gen[0] = g;
                ok[0] = true;
                latch.countDown();
            }

            @Override
            public void onError(String error) {
                setError("generator init failed: " + error);
                latch.countDown();
            }
        });
        try {
            if (!latch.await(INIT_TIMEOUT_SEC, TimeUnit.SECONDS)) {
                setError("generator init timed out after " + INIT_TIMEOUT_SEC + "s");
                return false;
            }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            setError("generator init interrupted");
            return false;
        }
        synchronized (mLock) {
            mGenerator = gen[0];
            return ok[0] && mGenerator != null;
        }
    }

    /**
     * Blocking: returns "playerPot\nstreamingPot\nvisitorData" or null.
     * Must NOT be called on the Android main thread (WebView callbacks need it).
     */
    public String fetchTokens(String videoId, String visitorData) {
        if (!ensureGenerator()) {
            return null;
        }
        PoTokenWebView gen;
        synchronized (mLock) {
            gen = mGenerator;
        }
        String vd = (visitorData == null || visitorData.isEmpty()) ? videoId : visitorData;
        String streamingPot = blockingToken(gen, vd);
        if (streamingPot == null) {
            return null;
        }
        String playerPot = blockingToken(gen, videoId);
        if (playerPot == null) {
            return null;
        }
        return playerPot + "\n" + streamingPot + "\n" + vd;
    }

    private String blockingToken(PoTokenWebView gen, String identifier) {
        final CountDownLatch latch = new CountDownLatch(1);
        final String[] out = new String[1];
        gen.generatePoToken(identifier, new PoTokenWebView.PoTokenCallback() {
            @Override
            public void onToken(String poToken) {
                out[0] = poToken;
                latch.countDown();
            }

            @Override
            public void onError(String error) {
                setError("token failed for " + identifier + ": " + error);
                latch.countDown();
            }
        });
        try {
            if (!latch.await(TOKEN_TIMEOUT_SEC, TimeUnit.SECONDS)) {
                setError("token timed out for " + identifier);
                return null;
            }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            setError("token mint interrupted for " + identifier);
            return null;
        }
        return out[0];
    }
}
