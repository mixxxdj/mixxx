#include "soundsourceproviderregistry.h"

#include "util/regex.h"

namespace Mixxx {

void SoundSourceProviderRegistry::registerProviderPlugin(
        const SoundSourceProviderPointer& pProvider,
        const SoundSourcePluginLibraryPointer& pPluginLibrary) {
    DEBUG_ASSERT(pProvider);
    Entry entry;
    entry.pProvider = pProvider;
    entry.pPluginLibrary = pPluginLibrary;
    const QStringList supportedFileTypes(
            pProvider->getSupportedFileTypes());
    DEBUG_ASSERT(pPluginLibrary || !supportedFileTypes.isEmpty());
    if (pPluginLibrary && supportedFileTypes.isEmpty()) {
        qWarning() << "SoundSource plugin does not support any file types"
                << pPluginLibrary->getFileName();
    }
    foreach (const QString& supportedFileType, supportedFileTypes) {
        m_entries.insert(supportedFileType, entry);
    }

    QRegExp().swap(m_supportedFileNameRegex); // invalidate
}

QStringList SoundSourceProviderRegistry::getSupportedPluginFileTypes() const {
    QSet<QString> supportedFileTypes;
    for (FileType2Entry::ConstIterator
            i(m_entries.begin()); m_entries.end() != i; ++i) {
        const SoundSourcePluginLibraryPointer pPluginLibrary(
                i.value().pPluginLibrary);
        if (i.value().pPluginLibrary) {
            supportedFileTypes += QSet<QString>::fromList(
                    i.value().pProvider->getSupportedFileTypes());
        }
    }
    return supportedFileTypes.toList();
}

QStringList SoundSourceProviderRegistry::getSupportedFileNamePatterns() const {
    const QStringList supportedFileTypes(getSupportedFileTypes());
    // Turn the list into a "*.mp3 *.wav *.etc" style string
    QStringList supportedFileNamePatterns;
    foreach(const QString& supportedFileType, supportedFileTypes) {
        supportedFileNamePatterns += QString("*.%1").arg(supportedFileType);
    }
    return supportedFileNamePatterns;
}

QRegExp SoundSourceProviderRegistry::getSupportedFileNameRegex() const {
    if (m_supportedFileNameRegex.isEmpty()) {
        // lazy initialization
        const QStringList supportedFileTypes(getSupportedFileTypes());
        QRegExp(
                RegexUtils::fileExtensionsRegex(supportedFileTypes),
                Qt::CaseInsensitive).swap(m_supportedFileNameRegex);
    }
    return m_supportedFileNameRegex;
}

} // Mixxx
