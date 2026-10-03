import Alpine from "alpinejs";
import focus from "@alpinejs/focus";
import persist from "@alpinejs/persist";
import collapse from "@alpinejs/collapse";

import Copy from "copy-text-to-clipboard";
window.Copy = Copy;

// TODO: date gesture
// import dayjs from 'dayjs';
// import customParseFormat from 'dayjs/plugin/customParseFormat'
// dayjs.extend(customParseFormat)

import "iconify-icon";

/*
import ApiFiles from './api/files.js'
import ApiForum from './api/forum.js'
import ApiUsers from './api/users.js'

window.api = {
    files: ApiFiles,
    forum: ApiForum,
    users: ApiUsers,
}
*/

Alpine.plugin(collapse);
Alpine.plugin(focus);
Alpine.plugin(persist);

import app from "./app.js";
Alpine.data("app", app);

import x_comp_knob from "./x-components/knob.js";
Alpine.data("knob", x_comp_knob);

// Must be exists in style.css
const daisyuiThemes = [
    "newmetro",
    "corporate",
    "cmyk",
    "emerald",
    "light",
    "dark",
    "abyss",
    "business",
    "dim",
    "dracula",
    "forest",
    "halloween",
    "sunset",
];

Alpine.store("theme", {
    current: Alpine.$persist("light").as("theme_current"),
    options: daisyuiThemes,
    init() {
        if (!this.options.includes(this.current)) {
            this.current = this.options[0];
        }
    },
});

window.Alpine = Alpine;

Alpine.start();

// Development:
//   > npx vite
// Build with:
//   > npx vite build
