#include "soundsourceplugin.h"
#include "defs_version.h"


extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT const char* Mixxx_getVersion() {
    return VERSION;
}

extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT int Mixxx_SoundSourcePluginAPI_getVersion() {
    return MIXXX_SOUNDSOURCEPLUGINAPI_VERSION;
}
