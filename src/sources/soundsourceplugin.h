#ifndef MIXXX_SOUNDSOURCEPLUGIN_H
#define MIXXX_SOUNDSOURCEPLUGIN_H

#include "sources/soundsourceprovider.h"
#include "sources/soundsourcepluginapi.h"

namespace Mixxx {

// Common base class for SoundSource plugins
class SoundSourcePlugin: public SoundSource {
protected:
    inline explicit SoundSourcePlugin(const QUrl& url)
            : SoundSource(url) {
    }
    inline SoundSourcePlugin(const QUrl& url, const QString& type)
            : SoundSource(url, type) {
    }
};

} // namespace Mixxx

extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT const char* Mixxx_getVersion();
extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT int Mixxx_SoundSourcePluginAPI_getVersion();

#endif
