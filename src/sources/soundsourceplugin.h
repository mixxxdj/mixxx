#ifndef MIXXX_SOUNDSOURCEPLUGIN_H
#define MIXXX_SOUNDSOURCEPLUGIN_H

#include "sources/soundsource.h"

namespace Mixxx {

// Common base class for SoundSource plugins
class SoundSourcePlugin: public SoundSource {
public:
    static char** allocFileExtensions(
            const QList<QString>& supportedFileExtensions);
    static void freeFileExtensions(char** fileExtensions);

protected:
    inline explicit SoundSourcePlugin(QString sFilename) :
            SoundSource(sFilename) {
    }
    inline SoundSourcePlugin(QString sFilename, QString sType) :
            SoundSource(sFilename, sType) {
    }
};

} // namespace Mixxx

#endif
