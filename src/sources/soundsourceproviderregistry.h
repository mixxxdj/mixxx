#ifndef MIXXX_SOUNDSOURCEPROVIDERREGISTRY_H
#define MIXXX_SOUNDSOURCEPROVIDERREGISTRY_H

#include "sources/soundsourcepluginlibrary.h"

#include <QMap>
#include <QRegExp>

namespace Mixxx {

// Registry for SoundSourceProviders
class SoundSourceProviderRegistry {
public:
    SoundSourceProviderPointer registerProvider(
            const SoundSourceProviderPointer& pProvider);
    SoundSourceProviderPointer registerPluginLibrary(
            const SoundSourcePluginLibraryPointer& pPluginLibrary);

    // Completes the registration by building the corresponding
    // regular expressions for file names.
    void finishRegistration();

    SoundSourceProviderPointer getProviderForFileExtension(
            const QString& fileExtension) const {
        return m_entries.value(fileExtension).pProvider;
    }

    SoundSourcePluginLibraryPointer getPluginLibraryForFileExtension(
            const QString& fileExtension) const {
        return m_entries.value(fileExtension).pPluginLibrary;
    }

    QStringList getSupportedFileExtensions() const {
        return m_entries.keys();
    }

    QStringList getSupportedFileNamePatterns() const;

    QRegExp getSupportedFileNameRegex() const {
        return m_supportedFileNameRegex;
    }

private:
    struct Entry {
        SoundSourceProviderPointer pProvider;
        SoundSourcePluginLibraryPointer pPluginLibrary;
    };
    typedef QMap<QString, Entry> FileExtension2Entry;

    SoundSourceProviderPointer registerEntry(const Entry& entry);

    FileExtension2Entry m_entries;

    QRegExp m_supportedFileNameRegex;
};

} // namespace Mixxx

#endif // MIXXX_SOUNDSOURCEPROVIDERREGISTRY_H
