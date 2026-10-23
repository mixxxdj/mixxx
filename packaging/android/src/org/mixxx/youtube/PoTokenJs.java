package org.mixxx.youtube;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

/**
 * Pure-Java port of NewPipe's JavaScriptUtil.kt: parses BotGuard challenge /
 * integrity-token payloads and formats values as JS Uint8Array literals.
 */
public final class PoTokenJs {
    private PoTokenJs() {
    }

    /** Parse Create-endpoint challenge into a JSON object embeddable in JS. */
    public static String parseChallengeData(String raw) throws Exception {
        JSONArray scrambled = new JSONArray(raw);
        JSONArray challengeData;
        if (scrambled.length() > 1 && scrambled.opt(1) instanceof String) {
            String descrambled = descramble(scrambled.getString(1));
            challengeData = new JSONArray(descrambled);
        } else {
            challengeData = scrambled.getJSONArray(0);
        }
        String messageId = challengeData.getString(0);
        String interpreterHash = challengeData.getString(3);
        String program = challengeData.getString(4);
        String globalName = challengeData.getString(5);
        String experimentsBlob = challengeData.getString(7);

        String safeScript = null;
        String trustedUrl = null;
        JSONArray arr1 = challengeData.optJSONArray(1);
        if (arr1 != null) {
            for (int i = 0; i < arr1.length(); i++) {
                if (arr1.opt(i) instanceof String) {
                    safeScript = arr1.getString(i);
                    break;
                }
            }
        }
        JSONArray arr2 = challengeData.optJSONArray(2);
        if (arr2 != null) {
            for (int i = 0; i < arr2.length(); i++) {
                if (arr2.opt(i) instanceof String) {
                    trustedUrl = arr2.getString(i);
                    break;
                }
            }
        }

        JSONObject interp = new JSONObject();
        if (safeScript != null) {
            interp.put("privateDoNotAccessOrElseSafeScriptWrappedValue", safeScript);
        } else {
            interp.put("privateDoNotAccessOrElseSafeScriptWrappedValue", JSONObject.NULL);
        }
        if (trustedUrl != null) {
            interp.put("privateDoNotAccessOrElseTrustedResourceUrlWrappedValue", trustedUrl);
        } else {
            interp.put("privateDoNotAccessOrElseTrustedResourceUrlWrappedValue", JSONObject.NULL);
        }

        JSONObject out = new JSONObject();
        out.put("messageId", messageId);
        out.put("interpreterJavascript", interp);
        out.put("interpreterHash", interpreterHash);
        out.put("program", program);
        out.put("globalName", globalName);
        out.put("clientExperimentsStateBlob", experimentsBlob);
        return out.toString();
    }

    /**
     * Parse GenerateIT payload. Returns [Uint8Array literal, ttl seconds].
     */
    public static String[] parseIntegrityTokenData(String raw) throws Exception {
        JSONArray arr = new JSONArray(raw);
        String tokenB64 = arr.getString(0);
        long ttl = arr.getLong(1);
        return new String[] { base64ToU8(tokenB64), Long.toString(ttl) };
    }

    /** String -> "new Uint8Array([...])" literal. */
    public static String stringToU8(String s) {
        return newUint8Array(s.getBytes(java.nio.charset.StandardCharsets.UTF_8));
    }

    /** "97,98,99" (Uint8Array.toString) -> base64url no padding. */
    public static String u8ToBase64(String csv) throws Exception {
        String[] parts = csv.split(",");
        byte[] raw = new byte[parts.length];
        for (int i = 0; i < parts.length; i++) {
            raw[i] = (byte) (Integer.parseInt(parts[i].trim()) & 0xFF);
        }
        return PoTokenWebView.base64UrlNoPad(raw);
    }

    private static String base64ToU8(String b64) throws Exception {
        return newUint8Array(base64ToBytes(b64));
    }

    private static String newUint8Array(byte[] contents) {
        StringBuilder sb = new StringBuilder("new Uint8Array([");
        for (int i = 0; i < contents.length; i++) {
            if (i != 0) {
                sb.append(',');
            }
            sb.append(contents[i] & 0xFF);
        }
        return sb.append("])").toString();
    }

    private static byte[] base64ToBytes(String b64) throws Exception {
        String mod = b64.replace('-', '+').replace('_', '/').replace('.', '=');
        byte[] decoded = android.util.Base64.decode(mod, android.util.Base64.DEFAULT);
        if (decoded == null) {
            throw new Exception("Cannot base64 decode");
        }
        return decoded;
    }

    private static String descramble(String scrambled) throws Exception {
        byte[] raw = base64ToBytes(scrambled);
        byte[] out = new byte[raw.length];
        for (int i = 0; i < raw.length; i++) {
            out[i] = (byte) (raw[i] + 97);
        }
        return new String(out, java.nio.charset.StandardCharsets.UTF_8);
    }
}
