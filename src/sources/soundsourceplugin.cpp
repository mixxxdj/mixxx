#include "soundsourceplugin.h"
#include "defs_version.h"


namespace mixxx {

void deleteSoundSourcePlugin(SoundSource* pSoundSource) {
    // The SoundSource must be deleted from within the external
    // library that has allocated it.
    delete pSoundSource;
}

} // namespace mixxx

extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT const char* Mixxx_getVersion() {
    return MIXXX_VERSION;
}

extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT int Mixxx_SoundSourcePluginAPI_getVersion() {
    return MIXXX_SOUNDSOURCEPLUGINAPI_VERSION;
}
