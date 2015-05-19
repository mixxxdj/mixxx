#include "soundsourceproviderregistry.h"

#include "util/regex.h"

namespace Mixxx {

SoundSourceProviderPointer SoundSourceProviderRegistry::registerProvider(
        const SoundSourceProviderPointer& pProvider) {
    Entry entry;
    entry.pProvider = pProvider;
    return registerEntry(entry);
}

SoundSourceProviderPointer SoundSourceProviderRegistry::registerPluginLibrary(
        const SoundSourcePluginLibraryPointer& pPluginLibrary) {
    Entry entry;
    entry.pProvider = pPluginLibrary->getSoundSourceProvider();
    entry.pPluginLibrary = pPluginLibrary;
    return registerEntry(entry);
}

SoundSourceProviderPointer SoundSourceProviderRegistry::registerEntry(const Entry& entry) {
    DEBUG_ASSERT(m_supportedFileNameRegex.isEmpty());
    DEBUG_ASSERT(entry.pProvider);
    const QStringList supportedFileTypes(
            entry.pProvider->getSupportedFileTypes());
    DEBUG_ASSERT(entry.pPluginLibrary || !supportedFileTypes.isEmpty());
    if (entry.pPluginLibrary && supportedFileTypes.isEmpty()) {
        qWarning() << "SoundSource plugin does not support any file types"
                << entry.pPluginLibrary->getFileName();
    }
    foreach (const QString& supportedFileType, supportedFileTypes) {
        m_entries.insert(supportedFileType, entry);
    }
    return entry.pProvider;
}

void SoundSourceProviderRegistry::finishRegistration() {
    const QStringList supportedFileTypes(getSupportedFileTypes());
    const QString fileExtensionsRegex(
            RegexUtils::fileExtensionsRegex(supportedFileTypes));
    QRegExp(fileExtensionsRegex, Qt::CaseInsensitive).swap(
            m_supportedFileNameRegex);
}

QStringList SoundSourceProviderRegistry::getSupportedFileNamePatterns() const {
    const QStringList supportedFileTypes(getSupportedFileTypes());
    // Turn the list into a "*.mp3 *.wav *.etc" style string
    QStringList supportedFileNamePatterns;
    foreach (const QString& supportedFileType, supportedFileTypes) {
        supportedFileNamePatterns += QString("*.%1").arg(supportedFileType);
    }
    return supportedFileNamePatterns;
}

} // Mixxx
