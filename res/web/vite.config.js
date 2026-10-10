import {defineConfig} from "vite";
import {join, resolve} from "path";

import tailwindcss from "@tailwindcss/vite";

export default defineConfig({
    // root: 'src' // does madness

    build: {
    // filenameHashing: false,
        manifest: true,
        emptyOutDir: true,
        outDir: resolve(__dirname, "dist"), // outDir: 'dist',

        // Uncomment for debug
        // minify: false,

        rollupOptions: {
            input: {
                main: join(__dirname, "src/main.js"),
                styles: join(__dirname, "src/styles.css"),
            },
        },
    },

    // https://vite.dev/config/server-options.html
    server: {
        port: 3003,
        cors: true,
    },

    plugins: [
        tailwindcss(), // v.4
    ],
});
