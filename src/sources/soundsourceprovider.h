#ifndef MIXXX_SOUNDSOURCEPROVIDER_H
#define MIXXX_SOUNDSOURCEPROVIDER_H

#include <QString>
#include <QStringList>
#include <QUrl>

#include "sources/soundsource.h"

namespace Mixxx {

// Factory interface for SoundSources
class SoundSourceProvider {
public:
    virtual ~SoundSourceProvider() {}

    virtual QString getName() const = 0;

    virtual QStringList getSupportedFileExtensions() const = 0;

    virtual SoundSourcePointer newSoundSource(const QUrl& url) = 0;
};

} // namespace Mixxx

#endif // MIXXX_SOUNDSOURCEPROVIDER_H
