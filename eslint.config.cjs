const jsdoc = require("eslint-plugin-jsdoc");
const typescriptEslint = require("@typescript-eslint/eslint-plugin");
const tsParser = require("@typescript-eslint/parser");
const js = require("@eslint/js");

const {
    FlatCompat,
} = require("@eslint/eslintrc");

const compat = new FlatCompat({
    baseDirectory: __dirname,
    recommendedConfig: js.configs.recommended,
    allConfig: js.configs.all
});

module.exports = [{
    ignores: [
        "res/controllers/lodash.mixxx.js",
        "res/controllers/Novation-Launchpad MK2-scripts.js",
        "res/controllers/Novation-Launchpad Mini MK3-scripts.js",
        "res/controllers/Novation-Launchpad-scripts.js",
    ],
}, ...compat.extends("eslint:recommended", "plugin:jsdoc/recommended", "plugin:diff/diff"), {
    plugins: {
        jsdoc,
        "@typescript-eslint": typescriptEslint,
    },

    languageOptions: {
        globals: {
            console: "readonly",
        },

        parser: tsParser,
        ecmaVersion: 7,
        sourceType: "script",
    },

    settings: {
        jsdoc: {
            preferredTypes: {
                object: "Object",
            },
        },
    },

    rules: {
        "array-bracket-spacing": "warn",
        "block-spacing": "warn",

        "brace-style": ["warn", "1tbs", {
            allowSingleLine: true,
        }],

        curly: "warn",
        camelcase: "warn",
        "comma-spacing": "warn",

        "computed-property-spacing": ["warn", "never", {
            enforceForClassMembers: true,
        }],

        "dot-location": ["warn", "property"],
        "dot-notation": "warn",
        eqeqeq: ["error", "always"],
        "func-call-spacing": "warn",

        "func-style": ["error", "expression", {
            allowArrowFunctions: true,
        }],

        indent: ["warn", 4],
        "key-spacing": "warn",
        "keyword-spacing": "warn",
        "linebreak-style": ["warn", "unix"],
        "newline-per-chained-call": "warn",
        "no-constructor-return": "warn",
        "no-extra-bind": "warn",
        "no-sequences": "warn",
        "no-useless-call": "warn",
        "no-useless-return": "warn",
        "no-trailing-spaces": "warn",

        "no-unneeded-ternary": ["warn", {
            defaultAssignment: false,
        }],

        "no-unused-vars": ["error", {
            argsIgnorePattern: "^_",
        }],

        "no-var": "warn",

        "object-curly-newline": ["warn", {
            consistent: true,
            multiline: true,
        }],

        "object-curly-spacing": "warn",
        "prefer-const": "warn",
        "prefer-regex-literals": "warn",
        "prefer-template": "warn",
        quotes: ["warn", "double"],
        "require-atomic-updates": "error",
        semi: "warn",
        "semi-spacing": "warn",
        "space-before-blocks": ["warn", "always"],
        "space-before-function-paren": ["warn", "never"],
        "space-in-parens": "warn",
        yoda: "warn",
    },
}, ...compat.extends("plugin:@typescript-eslint/recommended").map(config => ({
    ...config,
    files: ["res/controllers/*.d.ts"],
})), {
    files: ["res/controllers/*.d.ts"],

    rules: {
        "no-unused-vars": "off",
    },
}, {
    files: ["**/*.mjs"],

    languageOptions: {
        ecmaVersion: 5,
        sourceType: "module",
    },
}, {
    files: ["res/controllers/common-hid-packet-parser.js"],

    languageOptions: {
        globals: {
            console: "readonly",
        },
    },

    rules: {
        camelcase: "off",
    },
}, {
    files: ["res/controllers/*.js"],
    ignores: ["res/controllers/common-hid-packet-parser.js"],

    languageOptions: {
        globals: {
            console: "readonly",
            svg: "writable",
            HIDController: "writable",
            HIDDebug: "writable",
            HIDPacket: "writable",
            controller: "writable",
        },
    },
}];
