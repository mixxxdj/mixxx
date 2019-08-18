#ifndef MIXXX_SOUNDSOURCEPROVIDER_H
#define MIXXX_SOUNDSOURCEPROVIDER_H

#include <QString>
#include <QStringList>
#include <QUrl>

#include "sources/soundsource.h"

namespace mixxx {

// Providers for the same file extension are selected according
// to the priority for which they have been registered. Only
// a single provider will be registered for each file extension
// and priority.
enum class SoundSourceProviderPriority {
    LOWEST,
    LOWER,
    DEFAULT,
    HIGHER,
    HIGHEST
};

// Factory interface for SoundSources
//
// The implementation of a SoundSourceProvider must be thread-safe, because
// a single instance might be accessed concurrently from different threads.
class SoundSourceProvider {
  public:
    virtual ~SoundSourceProvider() {
    }

    // A user-readable name that identifies this provider.
    virtual QString getName() const = 0;

    // A list of supported file extensions in any order.
    virtual QStringList getSupportedFileExtensions() const = 0;

    // The default cooperative priority of this provider compared to
    // others supporting the same file extension(s). Please note that
    // an application may override the returned value to support
    // customization.
    //
    // The priority may vary with the file type that is currently
    // represented by the file extension.
    virtual SoundSourceProviderPriority getPriorityHint(
            const QString& supportedFileExtension) const {
        Q_UNUSED(supportedFileExtension);
        return SoundSourceProviderPriority::DEFAULT;
    }

    // Creates a new SoundSource for the file referenced by the URL.
    // This function should return a nullptr pointer if it is already
    // able to decide that the file is not supported even though it
    // has one of the supported file extensions.
    virtual SoundSourcePointer newSoundSource(const QUrl& url) = 0;
};

typedef std::shared_ptr<SoundSourceProvider> SoundSourceProviderPointer;

} // namespace mixxx

#endif // MIXXX_SOUNDSOURCEPROVIDER_H
