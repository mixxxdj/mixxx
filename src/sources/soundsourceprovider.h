#ifndef MIXXX_SOUNDSOURCEPROVIDER_H
#define MIXXX_SOUNDSOURCEPROVIDER_H

#include "sources/soundsource.h"

#include <QStringList>

namespace Mixxx {

// Factory interface for SoundSources
class SoundSourceProvider {
public:
    virtual ~SoundSourceProvider() {}

    virtual QString getName() const = 0;

    virtual QStringList getSupportedFileTypes() const = 0;

    virtual SoundSourcePointer newSoundSource(const QUrl& url) = 0;
};

typedef QSharedPointer<SoundSourceProvider> SoundSourceProviderPointer;

} // namespace Mixxx

#endif // MIXXX_SOUNDSOURCEPROVIDER_H
