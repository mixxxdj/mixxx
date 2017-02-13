#include <QApplication>
#include <QDesktopServices>

#include "sources/soundsourceproxy.h"

#ifdef __MAD__
#include "sources/soundsourcemp3.h"
#endif
#include "sources/soundsourceoggvorbis.h"
#ifdef __OPUS__
#include "sources/soundsourceopus.h"
#endif
#ifdef __COREAUDIO__
#include "sources/soundsourcecoreaudio.h"
#endif
#ifdef __SNDFILE__
#include "sources/soundsourcesndfile.h"
#endif
#ifdef __FFMPEGFILE__
#include "sources/soundsourceffmpeg.h"
#endif
#ifdef __MODPLUG__
#include "sources/soundsourcemodplug.h"
#endif
#include "sources/soundsourceflac.h"

#include "library/coverartutils.h"
#include "library/coverartcache.h"
#include "util/cmdlineargs.h"
#include "util/regex.h"

//Static memory allocation
/*static*/ mixxx::SoundSourceProviderRegistry SoundSourceProxy::s_soundSourceProviders;
/*static*/ QStringList SoundSourceProxy::s_supportedFileNamePatterns;
/*static*/ QRegExp SoundSourceProxy::s_supportedFileNamesRegex;

namespace {

#if (__UNIX__ || __LINUX__ || __APPLE__)
// Filtering of plugin file names on UNIX systems
const QStringList SOUND_SOURCE_PLUGIN_FILENAME_PATTERN("libsoundsource*");
#else
// No filtering of plugin file names on other systems, e.g. Windows
const QStringList SOUND_SOURCE_PLUGIN_FILENAME_PATTERN; // empty
#endif

QList<QDir> getSoundSourcePluginDirectories() {
    QList<QDir> pluginDirs;

    const QString& pluginPath = CmdlineArgs::Instance().getPluginPath();
    if (!pluginPath.isEmpty()) {
        qDebug() << "Adding plugin path from commandline arg:" << pluginPath;
        pluginDirs << QDir(pluginPath);
    }

    const QString dataLocation = QDesktopServices::storageLocation(
            QDesktopServices::DataLocation);
    const QString applicationPath = QCoreApplication::applicationDirPath();

#ifdef __LINUX__
    // TODO(rryan): Why can't we use applicationDirPath() and assume it's in the
    // 'bin' folder of $PREFIX, so we just traverse
    // ../lib/mixxx/plugins/soundsource.
    QDir libPluginDir(UNIX_LIB_PATH);
    if (libPluginDir.cd("plugins") && libPluginDir.cd("soundsource")) {
        pluginDirs << libPluginDir;
    }

    QDir dataPluginDir(dataLocation);
    if (dataPluginDir.cd("plugins") && dataPluginDir.cd("soundsource")) {
        pluginDirs << dataPluginDir;
    }

    // For people who build from source.
    QDir developer32Root(applicationPath);
    if (developer32Root.cd("lin32_build") && developer32Root.cd("plugins")) {
        pluginDirs << developer32Root.absolutePath();
    }
    QDir developer64Root(applicationPath);
    if (developer64Root.cd("lin64_build") && developer64Root.cd("plugins")) {
        pluginDirs << developer64Root.absolutePath();
    }
#elif __WINDOWS__
    QDir appPluginDir(applicationPath);
    if (appPluginDir.cd("plugins") && appPluginDir.cd("soundsource")) {
        pluginDirs << appPluginDir;
    }
#elif __APPLE__
    // blah/Mixxx.app/Contents/MacOS/../PlugIns/
    // TODO(XXX): Our SCons bundle target doesn't handle plugin subdirectories
    // :( so we can't do:
    //blah/Mixxx.app/Contents/PlugIns/soundsource
    QDir bundlePluginDir(applicationPath);
    if (bundlePluginDir.cdUp() && bundlePluginDir.cd("PlugIns")) {
        pluginDirs << bundlePluginDir;
    }

    // For people who build from source.
    QDir developer32Root(applicationPath);
    if (developer32Root.cd("osx32_build") && developer32Root.cd("plugins")) {
        pluginDirs << developer32Root.absolutePath();
    }
    QDir developer64Root(applicationPath);
    if (developer64Root.cd("osx64_build") && developer64Root.cd("plugins")) {
        pluginDirs << developer64Root.absolutePath();
    }

    QDir dataPluginDir(dataLocation);
    if (dataPluginDir.cd("Plugins") && dataPluginDir.cd("soundsource")) {
        pluginDirs << dataPluginDir;
    }
#endif

    return pluginDirs;
}

QUrl getCanonicalUrlForTrack(const Track* pTrack) {
    if (pTrack == nullptr) {
        // Missing track
        return QUrl();
    }
    const QString canonicalLocation(pTrack->getCanonicalLocation());
    if (canonicalLocation.isEmpty()) {
        // Corresponding file is missing or inaccessible
        //
        // NOTE(uklotzde): Special case handling is required for Qt 4.8!
        // Creating an URL from an empty local file in Qt 4.8 will result
        // in an URL with the string "file:" instead of an empty URL.
        //
        // TODO(XXX): This is no longer required for Qt 5.x
        // http://doc.qt.io/qt-5/qurl.html#fromLocalFile
        // "An empty localFile leads to an empty URL (since Qt 5.4)."
        return QUrl();
    }
    return QUrl::fromLocalFile(canonicalLocation);
}

} // anonymous namespace

// static
void SoundSourceProxy::loadPlugins() {
    // Initialize built-in file types.
    // Fallback providers should be registered before specialized
    // providers to ensure that they are only after the specialized
    // provider failed to open a file. But the order of registration
    // only matters among providers with equal priority.
#ifdef __FFMPEGFILE__
    // Use FFmpeg as the last resort.
    s_soundSourceProviders.registerProvider(
            std::make_shared<mixxx::SoundSourceProviderFFmpeg>());
#endif
#ifdef __SNDFILE__
    // libsndfile is another fallback
    s_soundSourceProviders.registerProvider(
            std::make_shared<mixxx::SoundSourceProviderSndFile>());
#endif
    s_soundSourceProviders.registerProvider(
            std::make_shared<mixxx::SoundSourceProviderFLAC>());
    s_soundSourceProviders.registerProvider(
            std::make_shared<mixxx::SoundSourceProviderOggVorbis>());
#ifdef __OPUS__
    s_soundSourceProviders.registerProvider(
            std::make_shared<mixxx::SoundSourceProviderOpus>());
#endif
#ifdef __MAD__
    s_soundSourceProviders.registerProvider(
            std::make_shared<mixxx::SoundSourceProviderMp3>());
#endif
#ifdef __MODPLUG__
    s_soundSourceProviders.registerProvider(
            std::make_shared<mixxx::SoundSourceProviderModPlug>());
#endif
#ifdef __COREAUDIO__
    s_soundSourceProviders.registerProvider(
            std::make_shared<mixxx::SoundSourceProviderCoreAudio>());
#endif

    // Scan for and initialize all plugins.
    // Loaded plugins will replace any built-in providers
    // that have been registered before (see above)!
    const QList<QDir> pluginDirs(getSoundSourcePluginDirectories());
    for (const auto& pluginDir: pluginDirs) {
        qDebug() << "Loading SoundSource plugins" << pluginDir.path();
        const QStringList files(pluginDir.entryList(
                SOUND_SOURCE_PLUGIN_FILENAME_PATTERN,
                QDir::Files | QDir::NoDotAndDotDot));
        for (const auto& file: files) {
            const QString libFilePath(pluginDir.filePath(file));
            mixxx::SoundSourcePluginLibraryPointer pPluginLibrary(
                    mixxx::SoundSourcePluginLibrary::load(libFilePath));
            if (pPluginLibrary) {
                s_soundSourceProviders.registerPluginLibrary(pPluginLibrary);
            } else {
                qWarning() << "Failed to load SoundSource plugin"
                        << libFilePath;
            }
        }
    }

    const QStringList supportedFileExtensions(
            s_soundSourceProviders.getRegisteredFileExtensions());
    for (const auto &supportedFileExtension: supportedFileExtensions) {
        qDebug() << "SoundSource providers for file extension" << supportedFileExtension;
        const QList<mixxx::SoundSourceProviderRegistration> registrationsForFileExtension(
                s_soundSourceProviders.getRegistrationsForFileExtension(
                        supportedFileExtension));
        for (const auto& registration: registrationsForFileExtension) {
            if (registration.getPluginLibrary()) {
                qDebug() << " " << static_cast<int>(registration.getProviderPriority())
                        << ":" << registration.getProvider()->getName()
                        << "@" << registration.getPluginLibrary()->getFilePath();
            } else {
                qDebug() << " " << static_cast<int>(registration.getProviderPriority())
                        << ":" << registration.getProvider()->getName();
            }
        }
    }

    // Turn the file extension list into a [ "*.mp3", "*.wav", ... ] style string list
    s_supportedFileNamePatterns.clear();
    for (const auto& supportedFileExtension: supportedFileExtensions) {
        s_supportedFileNamePatterns += QString("*.%1").arg(supportedFileExtension);
    }

    // Build regular expression of supported file extensions
    QString supportedFileExtensionsRegex(
            RegexUtils::fileExtensionsRegex(supportedFileExtensions));
    s_supportedFileNamesRegex =
            QRegExp(supportedFileExtensionsRegex, Qt::CaseInsensitive);
}

// static
QStringList SoundSourceProxy::getSupportedFileExtensionsByPlugins() {
    QStringList supportedFileExtensionsByPlugins;
    const QStringList supportedFileExtensions(getSupportedFileExtensions());
    for (const auto &supportedFileExtension: supportedFileExtensions) {
        const QList<mixxx::SoundSourceProviderRegistration> registrationsForFileExtension(
                s_soundSourceProviders.getRegistrationsForFileExtension(
                        supportedFileExtension));
        for (const auto& registration: registrationsForFileExtension) {
            if (registration.getPluginLibrary()) {
                supportedFileExtensionsByPlugins += supportedFileExtension;
            }
        }
    }
    return supportedFileExtensionsByPlugins;
}

// static
bool SoundSourceProxy::isUrlSupported(const QUrl& url) {
    const QFileInfo fileInfo(url.toLocalFile());
    return isFileSupported(fileInfo);
}

// static
bool SoundSourceProxy::isFileSupported(const QFileInfo& fileInfo) {
    return isFileNameSupported(fileInfo.fileName());
}

// static
bool SoundSourceProxy::isFileNameSupported(const QString& fileName) {
    return fileName.contains(getSupportedFileNamesRegex());
}

// static
bool SoundSourceProxy::isFileExtensionSupported(const QString& fileExtension) {
    return !s_soundSourceProviders.getRegistrationsForFileExtension(fileExtension).isEmpty();
}

// static
QList<mixxx::SoundSourceProviderRegistration>
SoundSourceProxy::findSoundSourceProviderRegistrations(
        const QUrl& url) {
    if (url.isEmpty()) {
        // silently ignore empty URLs
        return QList<mixxx::SoundSourceProviderRegistration>();
    }
    QString fileExtension(mixxx::SoundSource::getFileExtensionFromUrl(url));
    if (fileExtension.isEmpty()) {
        qWarning() << "Unknown file type:" << url.toString();
        return QList<mixxx::SoundSourceProviderRegistration>();
    }

    QList<mixxx::SoundSourceProviderRegistration> registrationsForFileExtension(
            s_soundSourceProviders.getRegistrationsForFileExtension(
                    fileExtension));
    if (registrationsForFileExtension.isEmpty()) {
        qWarning() << "Unsupported file type:" << url.toString();
    }

    return registrationsForFileExtension;
}

//static
SoundSourceProxy::SaveTrackMetadataResult SoundSourceProxy::saveTrackMetadata(
        const Track* pTrack,
        bool evenIfNeverParsedFromFileBefore) {
    DEBUG_ASSERT(nullptr != pTrack);
    SoundSourceProxy proxy(pTrack);
    if (proxy.m_pSoundSource) {
        mixxx::TrackMetadata trackMetadata;
        bool parsedFromFile = false;
        pTrack->getTrackMetadata(&trackMetadata, &parsedFromFile);
        if (parsedFromFile || evenIfNeverParsedFromFileBefore) {
            switch (proxy.m_pSoundSource->writeTrackMetadata(trackMetadata)) {
            case OK:
                qDebug() << "Track metadata has been written into file"
                        << pTrack->getLocation();
                return SaveTrackMetadataResult::SUCCEEDED;
            case ERR:
                break;
            default:
                DEBUG_ASSERT(!"unreachable code");
            }
        } else {
            qDebug() << "Skip writing of track metadata into file"
                    << pTrack->getLocation();
            return SaveTrackMetadataResult::SKIPPED;
        }
    }
    qDebug() << "Failed to write track metadata into file"
            << pTrack->getLocation();
    return SaveTrackMetadataResult::FAILED;
}

SoundSourceProxy::SoundSourceProxy(const TrackPointer& pTrack)
    : m_pTrack(pTrack),
      m_url(getCanonicalUrlForTrack(pTrack.get())),
      m_soundSourceProviderRegistrations(findSoundSourceProviderRegistrations(m_url)),
      m_soundSourceProviderRegistrationIndex(0) {
    initSoundSource();
}

SoundSourceProxy::SoundSourceProxy(const Track* pTrack)
    : m_url(getCanonicalUrlForTrack(pTrack)),
      m_soundSourceProviderRegistrations(findSoundSourceProviderRegistrations(m_url)),
      m_soundSourceProviderRegistrationIndex(0) {
    initSoundSource();
}

mixxx::SoundSourceProviderPointer SoundSourceProxy::getSoundSourceProvider() const {
    DEBUG_ASSERT(0 <= m_soundSourceProviderRegistrationIndex);
    if (m_soundSourceProviderRegistrations.size() > m_soundSourceProviderRegistrationIndex) {
        return m_soundSourceProviderRegistrations[m_soundSourceProviderRegistrationIndex].getProvider();
    } else {
        return mixxx::SoundSourceProviderPointer();
    }
}

void SoundSourceProxy::nextSoundSourceProvider() {
    if (m_soundSourceProviderRegistrations.size() > m_soundSourceProviderRegistrationIndex) {
        ++m_soundSourceProviderRegistrationIndex;
        // Discard SoundSource and AudioSource from previous provider
        closeAudioSource();
        m_pSoundSource = mixxx::SoundSourcePointer();
    }
}

void SoundSourceProxy::initSoundSource() {
    DEBUG_ASSERT(!m_pSoundSource);
    DEBUG_ASSERT(!m_pAudioSource);
    while (!m_pSoundSource) {
        mixxx::SoundSourceProviderPointer pProvider(getSoundSourceProvider());
        if (!pProvider) {
            if (!getUrl().isEmpty()) {
                qWarning() << "No SoundSourceProvider for file"
                           << getUrl().toString();
            }
            // Failure
            return;
        }
        m_pSoundSource = pProvider->newSoundSource(m_url);
        if (!m_pSoundSource) {
            qWarning() << "SoundSourceProvider"
                       << pProvider->getName()
                       << "failed to create a SoundSource for file"
                       << getUrl().toString();
            // Switch to next provider...
            nextSoundSourceProvider();
            // ...and continue loop
            DEBUG_ASSERT(!m_pSoundSource);
        } else {
            QString trackType(m_pSoundSource->getType());
            qDebug() << "SoundSourceProvider"
                     << pProvider->getName()
                     << "created a SoundSource for file"
                     << getUrl().toString()
                     << "of type"
                     << trackType;
            if (m_pTrack) {
                m_pTrack->setType(trackType);
            }
        }
    }
}

namespace {
    // Parses artist/title from the file name and returns the file type.
    // Assumes that the file name is written like: "artist - title.xxx"
    // or "artist_-_title.xxx".
    // This function does not overwrite any existing (non-empty) artist
    // and title fields!
    void parseMetadataFromFileName(mixxx::TrackMetadata* pTrackMetadata, QString fileName) {
        fileName.replace("_", " ");
        QString titleWithFileType;
        if (fileName.count('-') == 1) {
            if (pTrackMetadata->getArtist().isEmpty()) {
                const QString artist(fileName.section('-', 0, 0).trimmed());
                if (!artist.isEmpty()) {
                    pTrackMetadata->setArtist(artist);
                }
            }
            titleWithFileType = fileName.section('-', 1, 1).trimmed();
        } else {
            titleWithFileType = fileName.trimmed();
        }
        if (pTrackMetadata->getTitle().isEmpty()) {
            const QString title(titleWithFileType.section('.', 0, -2).trimmed());
            if (!title.isEmpty()) {
                pTrackMetadata->setTitle(title);
            }
        }
    }
} // anonymous namespace

void SoundSourceProxy::loadTrackMetadataAndCoverArt(
        bool withCoverArt,
        bool reloadFromFile) const {
    DEBUG_ASSERT(m_pTrack);

    if (!m_pSoundSource) {
        // Silently ignore requests for unsupported files
        qDebug() << "Unable to parse file tags without a SoundSource"
                 << getUrl().toString();
        return;
    }

    // Use the existing trackMetadata as default values. Otherwise
    // existing values in the library will be overwritten with
    // empty values if the corresponding file tags are missing.
    // Depending on the file type some kind of tags might even
    // not be supported at all and those would get lost!
    bool parsedFromFile = false;
    mixxx::TrackMetadata trackMetadata;
    m_pTrack->getTrackMetadata(&trackMetadata, &parsedFromFile);
    if (parsedFromFile && !reloadFromFile) {
        qDebug() << "Skip parsing of track metadata from file"
                 << getUrl().toString();
        return; // do not reload from file
    }

    // If parsing of the cover art image should be omitted the
    // 2nd output parameter must be set to nullptr. Cover art
    // is not reloaded from file once the metadata has been parsed!
    CoverInfoRelative coverInfoRelative;
    QImage coverImg;
    DEBUG_ASSERT(coverImg.isNull());
    QImage* pCoverImg = (withCoverArt && !parsedFromFile) ? &coverImg : nullptr;
    bool parsedCoverArt = false;

    // Parse the tags stored in the audio file.
    if (m_pSoundSource &&
            (m_pSoundSource->parseTrackMetadataAndCoverArt(&trackMetadata, pCoverImg) == OK)) {
        parsedFromFile = true;
        if (!coverImg.isNull()) {
            // Cover image has been parsed from the file
            // TODO() here we may introduce a duplicate hash code
            coverInfoRelative.hash = CoverArtUtils::calculateHash(coverImg);
            coverInfoRelative.coverLocation = QString();
            coverInfoRelative.type = CoverInfo::METADATA;
            coverInfoRelative.source = CoverInfo::GUESSED;
            parsedCoverArt = true;
        }
    } else {
        qWarning() << "Failed to parse track metadata from file"
                   << getUrl().toString();
        if (parsedFromFile) {
            // Don't overwrite any existing metadata that once has
            // been parsed successfully from file.
            return;
        }
    }

    // If Artist or title fields are blank try to parse them
    // from the file name.
    // TODO(rryan): Should we re-visit this decision?
    if (trackMetadata.getArtist().isEmpty() || trackMetadata.getTitle().isEmpty()) {
        parseMetadataFromFileName(&trackMetadata, m_pTrack->getFileInfo().fileName());
    }

    // Dump the trackMetadata extracted from the file back into the track.
    m_pTrack->setTrackMetadata(trackMetadata, parsedFromFile);
    if (parsedCoverArt) {
        m_pTrack->setCoverInfo(coverInfoRelative);
    }
}

Result SoundSourceProxy::parseTrackMetadata(mixxx::TrackMetadata* pTrackMetadata) const {
    if (m_pSoundSource) {
        return m_pSoundSource->parseTrackMetadataAndCoverArt(pTrackMetadata, nullptr);
    } else {
        return ERR;
    }
}

QImage SoundSourceProxy::parseCoverImage() const {
    QImage coverImg;
    if (m_pSoundSource) {
        m_pSoundSource->parseTrackMetadataAndCoverArt(nullptr, &coverImg);
    }
    return coverImg;
}

namespace {

// Keeps the TIO alive while accessing the audio data
// of the track. The TIO must not be deleted while
// accessing the corresponding file to avoid file
// corruption when writing metadata while the file
// is still in use.
class AudioSourceProxy: public mixxx::AudioSource {
public:
    AudioSourceProxy(
            const TrackPointer& pTrack,
            const mixxx::AudioSourcePointer& pAudioSource)
        : mixxx::AudioSource(*pAudioSource),
          m_pTrack(pTrack),
          m_pAudioSource(pAudioSource) {
    }
    AudioSourceProxy(const AudioSourceProxy&) = delete;
    AudioSourceProxy(AudioSourceProxy&&) = delete;

    static mixxx::AudioSourcePointer create(
            const TrackPointer& pTrack,
            const mixxx::AudioSourcePointer& pAudioSource) {
        DEBUG_ASSERT(pTrack);
        DEBUG_ASSERT(pAudioSource);
        return mixxx::AudioSourcePointer(
                std::make_shared<AudioSourceProxy>(pTrack, pAudioSource));
    }

    SINT seekSampleFrame(SINT frameIndex) override {
        return m_pAudioSource->seekSampleFrame(
                frameIndex);
    }

    SINT readSampleFrames(
            SINT numberOfFrames,
            CSAMPLE* sampleBuffer) override {
        return m_pAudioSource->readSampleFrames(
                numberOfFrames,
                sampleBuffer);
    }

    SINT readSampleFramesStereo(
            SINT numberOfFrames,
            CSAMPLE* sampleBuffer,
            SINT sampleBufferSize) override {
        return m_pAudioSource->readSampleFramesStereo(
                numberOfFrames,
                sampleBuffer,
                sampleBufferSize);
    }

private:
    const TrackPointer m_pTrack;
    const mixxx::AudioSourcePointer m_pAudioSource;
};

} // anonymous namespace

mixxx::AudioSourcePointer SoundSourceProxy::openAudioSource(const mixxx::AudioSourceConfig& audioSrcCfg) {
    DEBUG_ASSERT(m_pTrack);
    while (m_pSoundSource && !m_pAudioSource) {
        qDebug() << "Opening file"
                << getUrl().toString()
                << "with provider"
                << getSoundSourceProvider()->getName();
        const mixxx::SoundSource::OpenResult openResult =
                m_pSoundSource->open(audioSrcCfg);
        if (openResult == mixxx::SoundSource::OpenResult::ABORTED) {
            qWarning() << "Unable to open file"
                    << getUrl().toString()
                    << "with provider"
                    << getSoundSourceProvider()->getName();
            // Continue with the next SoundSource provider
            nextSoundSourceProvider();
            initSoundSource();
            continue; // try again
        }
        if ((openResult == mixxx::SoundSource::OpenResult::SUCCEEDED) && m_pSoundSource->verifyReadable()) {
            m_pAudioSource =
                    AudioSourceProxy::create(m_pTrack, m_pSoundSource);
            DEBUG_ASSERT(m_pAudioSource);
            if (m_pAudioSource->isEmpty()) {
                qWarning() << "File is empty"
                           << getUrl().toString();
            }
            // Overwrite metadata with actual audio properties
            if (m_pTrack) {
                DEBUG_ASSERT(m_pAudioSource->hasValidChannelCount());
                m_pTrack->setChannels(m_pAudioSource->getChannelCount());
                DEBUG_ASSERT(m_pAudioSource->hasValidSamplingRate());
                m_pTrack->setSampleRate(m_pAudioSource->getSamplingRate());
                if (m_pAudioSource->hasDuration()) {
                    // optional property
                    m_pTrack->setDuration(m_pAudioSource->getDuration());
                }
                if (m_pAudioSource->hasBitrate()) {
                    // optional property
                    m_pTrack->setBitrate(m_pAudioSource->getBitrate());
                }
            }
        } else {
            qWarning() << "Failed to open file"
                       << getUrl().toString()
                       << "with provider"
                       << getSoundSourceProvider()->getName();
            if (openResult == mixxx::SoundSource::OpenResult::SUCCEEDED) {
                m_pSoundSource->close(); // cleanup
            }
            // Do NOT retry with the next SoundSource provider if the file
            // itself is the cause!
            DEBUG_ASSERT(!m_pAudioSource);
        }
        return m_pAudioSource; // either success or failure
    }
    // All available providers have returned OpenResult::ABORTED when
    // getting here. m_pSoundSource might already be invalid/null!
    qWarning() << "Unable to decode file"
            << getUrl().toString();
    DEBUG_ASSERT(!m_pAudioSource);
    return m_pAudioSource;
}

void SoundSourceProxy::closeAudioSource() {
    if (m_pAudioSource) {
        DEBUG_ASSERT(m_pSoundSource);
        m_pSoundSource->close();
        m_pAudioSource = mixxx::AudioSourcePointer();
        qDebug() << "Closed AudioSource for file"
                 << getUrl().toString();
    }
}
