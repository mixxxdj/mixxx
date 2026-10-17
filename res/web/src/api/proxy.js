import {RCONTROL_URL} from "./endpoints.js";

/**
 * Read a cookie by name using modern HTML5 API.
 * @param {string} name
 * @returns {string|undefined}
 */
export function readCookie(name) {
    const match = document.cookie
        .split(/;\s*/)
        .find(c => c.startsWith(`${name  }=`));
    return match ? match.slice(name.length + 1) : undefined;
}

/**
 * Send an RControl command to the Mixxx backend via fetch.
 *
 * @param {object} commandObj - Command object, e.g. { getcrossfader: "true" }
 * @param {object} [options]
 * @param {boolean} [options.requireAuth] - If false, sessionid is omitted (e.g. login)
 * @returns {Promise<Array>} Parsed response from the backend
 */
export async function rcontrol(commandObj, {requireAuth = true} = {}) {
    const payload = [];

    if (requireAuth) {
        const sessionid = readCookie("sessionid");
        if (!sessionid) {
            window.location.replace("/chameleon/login.html");
            throw new Error("No session");
        }
        payload.push({sessionid});
    }

    payload.push(commandObj);

    const res = await fetch(RCONTROL_URL, {
        method: "POST",
        headers: {"Content-Type": "application/json"},
        body: JSON.stringify(payload),
    });

    if (!res.ok) {
        if (res.status === 401) {
            window.location.replace("/chameleon/login.html");
        }
        throw new Error(`RControl error: ${res.status}`);
    }

    return res.json();
}
