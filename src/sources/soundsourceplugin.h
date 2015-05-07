#ifndef MIXXX_SOUNDSOURCEPLUGIN_H
#define MIXXX_SOUNDSOURCEPLUGIN_H

#include "sources/soundsource.h"
#include "defs_version.h"

// Getter functions to be declared by all SoundSource plugins
typedef Mixxx::SoundSource* (*getSoundSourceFunc)(QUrl url);
typedef char** (*getSupportedFileExtensionsFunc)();
typedef int (*getSoundSourceAPIVersionFunc)();
/// New in version 3
typedef void (*freeFileExtensionsFunc)(char** fileExts);

namespace Mixxx {

// Common base class for SoundSource plugins
class SoundSourcePlugin: public SoundSource {
public:
    static char** allocFileExtensions(
            const QList<QString>& supportedFileExtensions);
    static void freeFileExtensions(char** fileExtensions);

protected:
    inline explicit SoundSourcePlugin(QUrl url)
            : SoundSource(url) {
    }
    inline SoundSourcePlugin(QUrl url, QString type)
            : SoundSource(url, type) {
    }
};

} // namespace Mixxx

#endif
