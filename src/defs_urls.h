#pragma once
#include "version.h"

// Two-level macro to stringize the numeric version definitions to a version
// string. See https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html for details.
#define TO_STR(x) #x
#define TO_VERSION_STR(major, minor) \
    TO_STR(major)                    \
    "." TO_STR(minor)

// Icons used for the main window and dialogs
#define MIXXX_ICON_PATH ":/images/icons/scalable/apps/mixxx.svg"
#define MIXXX_LOGO_PATH ":/images/mixxx_logo.svg"

#define MIXXX_WEBSITE_URL       "https://www.mixxx.org"
#define MIXXX_WEBSITE_SHORT_URL "www.mixxx.org"
#define MIXXX_SUPPORT_URL       "https://www.mixxx.org/support/"
#define MIXXX_TRANSLATION_URL   "https://www.transifex.com/projects/p/mixxxdj/"
#define MIXXX_DONATE_URL "https://mixxx.org/donate"

#define MIXXX_CONTROLLER_FORUMS_URL \
    "https://mixxx.discourse.group/c/controller-mappings/10"

#define MIXXX_WIKI_URL "https://github.com/mixxxdj/mixxx/wiki"
#define MIXXX_WIKI_TROUBLESHOOTING_SOUND_URL \
    MIXXX_WIKI_URL "/troubleshooting#i-cant-select-my-sound-card-in-the-sound-hardware-preferences"
#define MIXXX_WIKI_HARDWARE_COMPATIBILITY_URL \
    MIXXX_WIKI_URL "/Hardware-Compatibility"
#define MIXXX_WIKI_AUDIO_LATENCY_URL \
    MIXXX_WIKI_URL "/Adjusting-Audio-Latency"
#define MIXXX_WIKI_CONTROLLER_MAPPING_FORMAT_URL \
    MIXXX_WIKI_URL "/Midi-Controller-Mapping-File-Format"
#define MIXXX_WIKI_MIDI_SCRIPTING_URL \
    MIXXX_WIKI_URL "/Midi-Scripting"

#define MIXXX_MANUAL_URL                        \
    "https://manual.mixxx.org/" TO_VERSION_STR( \
            MIXXX_VERSION_MAJOR, MIXXX_VERSION_MINOR)
#define MIXXX_MANUAL_SHORTCUTS_URL \
    MIXXX_MANUAL_URL "/chapters/controlling_mixxx.html#using-a-keyboard"
#define MIXXX_MANUAL_COMMANDLINEOPTIONS_URL \
    MIXXX_MANUAL_URL "/chapters/appendix/commandline_dev_tools.html"
#define MIXXX_MANUAL_CONTROLLERS_URL \
    MIXXX_MANUAL_URL "/chapters/controlling_mixxx.html#using-midi-hid-controllers"
#define MIXXX_MANUAL_CONTROLLERMANUAL_PREFIX \
    MIXXX_MANUAL_URL "/hardware/controllers/"
#define MIXXX_MANUAL_CONTROLLERMANUAL_SUFFIX ".html"
#define MIXXX_MANUAL_CONTROLS_URL \
    MIXXX_MANUAL_URL "/chapters/advanced_topics.html#mixxx-controls"
#define MIXXX_MANUAL_SOUND_URL \
    MIXXX_MANUAL_URL "/chapters/preferences/sound_hardware.html"
#define MIXXX_MANUAL_SOUND_API_URL \
    MIXXX_MANUAL_URL "/chapters/preferences/sound_hardware.html#sound-api"
#define MIXXX_MANUAL_LIBRARY_URL \
    MIXXX_MANUAL_URL "/chapters/preferences.html#library"
#define MIXXX_MANUAL_CUE_MODES_URL \
    MIXXX_MANUAL_URL "/chapters/user_interface.html#using-cue-modes"
#define MIXXX_MANUAL_SYNC_MODES_URL \
    MIXXX_MANUAL_URL "/chapters/djing_with_mixxx#sync-lock-with-dynamic-tempo"
#define MIXXX_MANUAL_TRACK_SEARCH_URL \
    MIXXX_MANUAL_URL "/chapters/library.html#finding-tracks-search"
#define MIXXX_MANUAL_MIC_MONITOR_MODES_URL \
    MIXXX_MANUAL_URL "/chapters/microphones"
#define MIXXX_MANUAL_MIC_LATENCY_URL \
    MIXXX_MANUAL_URL "/chapters/microphones#latency-compensation"
#define MIXXX_MANUAL_BEATS_URL \
    MIXXX_MANUAL_URL "/chapters/preferences.html#beat-detection"
#define MIXXX_MANUAL_KEY_URL \
    MIXXX_MANUAL_URL "/chapters/preferences.html#key-detection"
#define MIXXX_MANUAL_EQ_URL \
    MIXXX_MANUAL_URL "/chapters/preferences.html#equalizers"
#define MIXXX_MANUAL_BROADCAST_URL \
    MIXXX_MANUAL_URL "/chapters/livebroadcasting.html#configuring-mixxx"
#define MIXXX_MANUAL_VINYL_URL \
    MIXXX_MANUAL_URL "/chapters/vinyl_control.html#configuring-vinyl-control"
#define MIXXX_MANUAL_VINYL_TROUBLESHOOTING_URL \
    MIXXX_MANUAL_URL "/chapters/vinyl_control.html#troubleshooting"
#define MIXXX_MANUAL_SETTINGS_DIRECTORY_URL \
    MIXXX_MANUAL_URL "/chapters/appendix.html#settings-directory"
#define MIXXX_MANUAL_FILENAME   "Mixxx-Manual.pdf"
#define MIXXX_KBD_SHORTCUTS_FILENAME "Mixxx-Keyboard-Shortcuts.pdf"
