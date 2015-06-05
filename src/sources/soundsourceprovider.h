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

    // A user-readable name that identifies this provider.
    virtual QString getName() const = 0;

    // A list of supported file extensions in any order.
    virtual QStringList getSupportedFileExtensions() const = 0;

    // Providers for the same file extension are selected according
    // to the priority for which they have been registered. Only
    // a single provider will be registered for each file extension
    // and priority.
    enum Priority {
        LOWEST_PRIORITY,
        LOWER_PRIORITY,
        DEFAULT_PRIORITY,
        HIGHER_PRIORITY,
        HIGHEST_PRIORITY
    };

    // The suggested priority of this provider compared to others
    // supporting the same file extension(s). Please note that an
    // application may register a provider with any priority, no
    // matter what this function actually returns!
    virtual Priority getPriorityHint() const {
        return DEFAULT_PRIORITY;
    }

    // Creates a new SoundSource for the file referenced by the URL.
    // This function should return a NULL pointer if it is already
    // able to decide that the file is not supported even though it
    // has one of the supported file extensions.
    virtual SoundSourcePointer newSoundSource(const QUrl& url) = 0;
};

} // namespace Mixxx

#endif // MIXXX_SOUNDSOURCEPROVIDER_H
