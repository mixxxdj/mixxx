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
    entry.pProvider = pPluginLibrary->createSoundSourceProvider();
    entry.pPluginLibrary = pPluginLibrary;
    return registerEntry(entry);
}

SoundSourceProviderPointer SoundSourceProviderRegistry::registerEntry(const Entry& entry) {
    DEBUG_ASSERT(m_supportedFileNameRegex.isEmpty());
    DEBUG_ASSERT(entry.pProvider);
    const QStringList supportedFileExtensions(
            entry.pProvider->getSupportedFileExtensions());
    DEBUG_ASSERT(entry.pPluginLibrary || !supportedFileExtensions.isEmpty());
    if (entry.pPluginLibrary && supportedFileExtensions.isEmpty()) {
        qWarning() << "SoundSource plugin does not support any file types"
                << entry.pPluginLibrary->getFilePath();
    }
    foreach (const QString& supportedFileExtension, supportedFileExtensions) {
        m_entries.insert(supportedFileExtension, entry);
    }
    return entry.pProvider;
}

void SoundSourceProviderRegistry::finishRegistration() {
    const QStringList supportedFileExtensions(getSupportedFileExtensions());
    const QString fileExtensionsRegex(
            RegexUtils::fileExtensionsRegex(supportedFileExtensions));
    QRegExp(fileExtensionsRegex, Qt::CaseInsensitive).swap(
            m_supportedFileNameRegex);
}

QStringList SoundSourceProviderRegistry::getSupportedFileNamePatterns() const {
    const QStringList supportedFileExtensions(getSupportedFileExtensions());
    // Turn the list into a "*.mp3 *.wav *.etc" style string
    QStringList supportedFileNamePatterns;
    foreach (const QString& supportedFileExtension, supportedFileExtensions) {
        supportedFileNamePatterns += QString("*.%1").arg(supportedFileExtension);
    }
    return supportedFileNamePatterns;
}

} // Mixxx
