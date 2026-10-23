package org.mixxx.youtube;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.util.Base64;
import android.util.Log;
import android.webkit.JavascriptInterface;
import android.webkit.WebSettings;
import android.webkit.WebView;

import org.json.JSONArray;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.time.Instant;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * Generates YouTube Proof-of-Origin (PO) tokens on-device using a headless
 * WebView that runs YouTube's BotGuard challenge — port of NewPipe's
 * potoken package (via ytdlnis) to plain Java, no RxJava/coroutines needed.
 *
 * Flow: load local po_token.html -> BotGuard Create -> run challenge in JS ->
 * GenerateIT (integrityToken) -> mint per-video PO tokens via obtainPoToken().
 * Tokens are ~6h valid; init runs once per process, token fetch per video id.
 */
public class PoTokenWebView {
    private static final String TAG = "PoTokenWebView";
    private static final String GOOGLE_API_KEY = "«redacted:AIza…»";
    private static final String REQUEST_KEY = "O43z0dpjhgX20SCx4KAo";
    private static final String USER_AGENT =
            "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
                    + "AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.3";
    private static final String JS_INTERFACE = "PoTokenWebView";

    private final Context mAppContext;
    private final WebView mWebView;
    private final ExecutorService mIo = Executors.newCachedThreadPool();
    private final Handler mMain = new Handler(Looper.getMainLooper());
    private final Map<String, PoTokenCallback> mPending = new HashMap<>();

    private volatile String mIntegrityTokenJs; // "new Uint8Array([...])" literal
    private volatile String mWebPoSignalOutputJs = "[]";
    private volatile Instant mExpiration = Instant.MIN;
    private volatile boolean mReady = false;
    private volatile InitCallback mInitCallback;

    public interface InitCallback {
        void onReady(PoTokenWebView gen);
        void onError(String error);
    }

    public interface PoTokenCallback {
        void onToken(String poToken);
        void onError(String error);
    }

    private PoTokenWebView(Context context) {
        mAppContext = context.getApplicationContext();
        mWebView = new WebView(mAppContext);
        WebSettings s = mWebView.getSettings();
        s.setJavaScriptEnabled(true);
        s.setUserAgentString(USER_AGENT);
        s.setBlockNetworkLoads(true); // WebView needs no internet
        mWebView.addJavascriptInterface(this, JS_INTERFACE);
    }

    /** Async init: loads BotGuard, runs challenge, obtains integrityToken. */
    public static void create(Context context, InitCallback cb) {
        final PoTokenWebView gen;
        try {
            gen = new PoTokenWebView(context);
        } catch (Exception e) {
            cb.onError("WebView unavailable: " + e.getMessage());
            return;
        }
        gen.mInitCallback = cb;
        new Handler(Looper.getMainLooper()).post(() -> gen.loadHtmlAndObtainBotguard());
    }

    public boolean isExpired() {
        return Instant.now().isAfter(mExpiration);
    }

    public boolean isReady() {
        return mReady && !isExpired();
    }

    /** Mint a PO token bound to identifier (video id for player, visitorData for GVS). */
    public void generatePoToken(final String identifier, final PoTokenCallback cb) {
        synchronized (mPending) {
            mPending.put(identifier, cb);
        }
        final String u8Identifier = PoTokenJs.stringToU8(identifier);
        mMain.post(() -> mWebView.evaluateJavascript(
                "try {\n"
                        + "  identifier = \"" + identifier + "\"\n"
                        + "  u8Identifier = " + u8Identifier + "\n"
                        + "  poTokenU8 = obtainPoToken(webPoSignalOutput, integrityToken, u8Identifier)\n"
                        + "  poTokenU8String = \"\"\n"
                        + "  for (i = 0; i < poTokenU8.length; i++) {\n"
                        + "    if (i != 0) poTokenU8String += \",\"\n"
                        + "    poTokenU8String += poTokenU8[i]\n"
                        + "  }\n"
                        + "  " + JS_INTERFACE + ".onObtainPoTokenResult(identifier, poTokenU8String)\n"
                        + "} catch (error) {\n"
                        + "  " + JS_INTERFACE + ".onObtainPoTokenError(identifier, error + \"\\n\" + error.stack)\n"
                        + "}",
                null));
    }

    @JavascriptInterface
    public void onObtainPoTokenResult(String identifier, String poTokenU8) {
        String poToken;
        try {
            poToken = PoTokenJs.u8ToBase64(poTokenU8);
        } catch (Exception e) {
            popPending(identifier, null, "base64 decode failed: " + e.getMessage());
            return;
        }
        popPending(identifier, poToken, null);
    }

    @JavascriptInterface
    public void onObtainPoTokenError(String identifier, String error) {
        Log.e(TAG, "obtainPoToken error: " + error);
        popPending(identifier, null, error);
    }

    private void popPending(String identifier, String token, String error) {
        PoTokenCallback cb;
        synchronized (mPending) {
            cb = mPending.remove(identifier);
        }
        if (cb == null) {
            return;
        }
        if (token != null) {
            cb.onToken(token);
        } else {
            cb.onError(error);
        }
    }

    private void loadHtmlAndObtainBotguard() {
        mIo.execute(() -> {
            try {
                InputStream in = mAppContext.getAssets().open("po_token.html");
                ByteArrayOutputStream bos = new ByteArrayOutputStream();
                byte[] buf = new byte[8192];
                int n;
                while ((n = in.read(buf)) > 0) {
                    bos.write(buf, 0, n);
                }
                in.close();
                final String html = new String(bos.toByteArray(), StandardCharsets.UTF_8);
                mMain.post(() -> mWebView.loadDataWithBaseURL(
                        "https://www.youtube.com",
                        html.replaceFirst("</script>",
                                "\n" + JS_INTERFACE + ".downloadAndRunBotguard()</script>"),
                        "text/html", "utf-8", null));
            } catch (Exception e) {
                failInit(e.getMessage());
            }
        });
    }

    @JavascriptInterface
    public void downloadAndRunBotguard() {
        mIo.execute(() -> {
            try {
                String challenge = postJson("https://www.youtube.com/api/jnn/v1/Create",
                        new JSONArray().put(REQUEST_KEY).toString());
                final String parsed = PoTokenJs.parseChallengeData(challenge);
                mMain.post(() -> mWebView.evaluateJavascript(
                        "try {\n"
                                + "  data = " + parsed + "\n"
                                + "  runBotGuard(data).then(function (result) {\n"
                                + "    this.webPoSignalOutput = result.webPoSignalOutput\n"
                                + "    " + JS_INTERFACE + ".onRunBotguardResult(result.botguardResponse)\n"
                                + "  }, function (error) {\n"
                                + "    " + JS_INTERFACE + ".onJsInitializationError(error + \"\\n\" + error.stack)\n"
                                + "  })\n"
                                + "} catch (error) {\n"
                                + "  " + JS_INTERFACE + ".onJsInitializationError(error + \"\\n\" + error.stack)\n"
                                + "}",
                        null));
            } catch (Exception e) {
                failInit(e.getMessage());
            }
        });
    }

    @JavascriptInterface
    public void onJsInitializationError(String error) {
        Log.e(TAG, "JS init error: " + error);
        failInit(error);
    }

    @JavascriptInterface
    public void onRunBotguardResult(String botguardResponse) {
        mIo.execute(() -> {
            try {
                String resp = postJson("https://www.youtube.com/api/jnn/v1/GenerateIT",
                        new JSONArray().put(REQUEST_KEY).put(botguardResponse).toString());
                String[] parsed = PoTokenJs.parseIntegrityTokenData(resp);
                mIntegrityTokenJs = parsed[0];
                long ttlSec = Long.parseLong(parsed[1]);
                mExpiration = Instant.now().plusSeconds(Math.max(0, ttlSec - 600));
                mMain.post(() -> mWebView.evaluateJavascript(
                        "this.integrityToken = " + mIntegrityTokenJs,
                        value -> {
                            mReady = true;
                            Log.i(TAG, "PO token generator ready, ttl=" + ttlSec + "s");
                            if (mInitCallback != null) {
                                mInitCallback.onReady(PoTokenWebView.this);
                            }
                        }));
            } catch (Exception e) {
                failInit(e.getMessage());
            }
        });
    }

    private void failInit(String error) {
        mMain.post(() -> {
            try {
                mWebView.loadUrl("about:blank");
                mWebView.onPause();
                mWebView.removeAllViews();
                mWebView.destroy();
            } catch (Exception ignored) {
            }
            if (mInitCallback != null) {
                mInitCallback.onError(error);
            }
        });
    }

    private static String postJson(String urlStr, String body) throws Exception {
        HttpURLConnection c = (HttpURLConnection) new URL(urlStr).openConnection();
        c.setRequestMethod("POST");
        c.setDoOutput(true);
        c.setConnectTimeout(20000);
        c.setReadTimeout(20000);
        c.setRequestProperty("User-Agent", USER_AGENT);
        c.setRequestProperty("Accept", "application/json");
        c.setRequestProperty("Content-Type", "application/json+protobuf");
        c.setRequestProperty("x-goog-api-key", GOOGLE_API_KEY);
        c.setRequestProperty("x-user-agent", "grpc-web-javascript/0.1");
        byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
        OutputStream os = c.getOutputStream();
        os.write(bytes);
        os.close();
        int code = c.getResponseCode();
        if (code != 200) {
            throw new Exception("BotGuard HTTP " + code);
        }
        InputStream in = c.getInputStream();
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        byte[] buf = new byte[8192];
        int n;
        while ((n = in.read(buf)) > 0) {
            bos.write(buf, 0, n);
        }
        in.close();
        return new String(bos.toByteArray(), StandardCharsets.UTF_8);
    }

    /** Base64url (no padding) for raw bytes — mirrors u8ToBase64 in PoTokenJs. */
    static String base64UrlNoPad(byte[] raw) {
        return Base64.encodeToString(raw, Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP);
    }
}
