import {rcontrol, readCookie} from "./proxy.js";
import {CMD} from "./endpoints.js";

/**
 * Login with the provided password.
 * On success, saves the sessionid cookie and redirects to index.html.
 *
 * @param {string} password
 */
export async function login(password) {
    const res = await rcontrol(
        {[CMD.LOGIN]: {password}},
        {requireAuth: false},
    );

    for (const item of res) {
        if (item.sessionid) {
            document.cookie = `sessionid=${  item.sessionid  }; SameSite=Lax; path=/`;
            window.location.replace("/chameleon/");
            return;
        }
    }
    throw new Error("Login failed");
}

/**
 * Check whether the current session is valid.
 * If invalid, clears the cookie and redirects to login.html.
 *
 * @returns {Promise<boolean>} true if authenticated
 */
export async function checkAuth() {
    const sessionid = readCookie("sessionid");
    if (!sessionid) {
        window.location.replace("/chameleon/login.html");
        return false;
    }

    try {
        const res = await rcontrol({sessionid}, {requireAuth: false});
        for (const item of res) {
            if (item.sessionid === sessionid) {
                return true; // valid session
            }
        }
        // Invalid session
        document.cookie = "sessionid=\"\"; SameSite=Lax; path=/";
        window.location.replace("/chameleon/login.html");
        return false;
    } catch {
        document.cookie = "sessionid=\"\"; SameSite=Lax; path=/";
        window.location.replace("/chameleon/login.html");
        return false;
    }
}
