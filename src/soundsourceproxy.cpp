#include <QApplication>
#include <QDesktopServices>

#include "soundsourceproxy.h"

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

#include "util/cmdlineargs.h"
#include "util/regex.h"

//static
Mixxx::SoundSourceProviderRegistry SoundSourceProxy::s_soundSourceProviders;

//static
QStringList SoundSourceProxy::s_supportedFileNamePatterns;

//static
QRegExp SoundSourceProxy::s_supportedFileNamesRegex;

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

} // anonymous namespace

// static
void SoundSourceProxy::loadPlugins() {
    // Initialize built-in file types.
    // Fallback providers should be registered before specialized
    // providers to ensure that they are only after the specialized
    // provider failed to open a file. But the order of registration
    // only matters among providers with equal priority.
#ifdef __FFMPEGFILE__
    // Use FFmpeg as the last resort
    s_soundSourceProviders.registerProvider(Mixxx::SoundSourceProviderPointer(
            new Mixxx::SoundSourceProviderFFmpeg));
#endif
#ifdef __SNDFILE__
    // libsndfile is another fallback
    s_soundSourceProviders.registerProvider(Mixxx::SoundSourceProviderPointer(
            new Mixxx::SoundSourceProviderSndFile));
#endif
    s_soundSourceProviders.registerProvider(Mixxx::SoundSourceProviderPointer(
            new Mixxx::SoundSourceProviderFLAC));
    s_soundSourceProviders.registerProvider(Mixxx::SoundSourceProviderPointer(
            new Mixxx::SoundSourceProviderOggVorbis));
#ifdef __OPUS__
    s_soundSourceProviders.registerProvider(Mixxx::SoundSourceProviderPointer(
            new Mixxx::SoundSourceProviderOpus));
#endif
#ifdef __MAD__
    s_soundSourceProviders.registerProvider(Mixxx::SoundSourceProviderPointer(
            new Mixxx::SoundSourceProviderMp3));
#endif
#ifdef __MODPLUG__
    s_soundSourceProviders.registerProvider(Mixxx::SoundSourceProviderPointer(
            new Mixxx::SoundSourceProviderModPlug));
#endif
#ifdef __COREAUDIO__
    s_soundSourceProviders.registerProvider(Mixxx::SoundSourceProviderPointer(
            new Mixxx::SoundSourceProviderCoreAudio));
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
            Mixxx::SoundSourcePluginLibraryPointer pPluginLibrary(
                    Mixxx::SoundSourcePluginLibrary::load(libFilePath));
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
        const auto pPluginLibrary(
                s_soundSourceProviders.getPluginLibraryForFileExtension(
                        supportedFileExtension));
        const auto pSoundSourceProvider(
                s_soundSourceProviders.getProviderForFileExtension(
                        supportedFileExtension));
        if (!pPluginLibrary.isNull()) {
            qDebug() << " " << pSoundSourceProvider->getName()
                    << "@" << pPluginLibrary->getFilePath();
        } else {
            qDebug() << " " << pSoundSourceProvider->getName();
        }
    }

    // Turn the list into a "*.mp3 *.wav *.etc" style string
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
        const auto pPluginLibrary(
                s_soundSourceProviders.getPluginLibraryForFileExtension(
                        supportedFileExtension));
        if (!pPluginLibrary.isNull()) {
            supportedFileExtensionsByPlugins += supportedFileExtension;
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
    return !s_soundSourceProviders.getProviderForFileExtension(fileExtension).isNull();
}

// static
Mixxx::SoundSourceProviderPointer
SoundSourceProxy::findSoundSourceProvider(
        const QUrl& url) {
    QString fileExtension(Mixxx::SoundSource::getFileExtensionFromUrl(url));
    if (fileExtension.isEmpty()) {
        qWarning() << "Unknown file type:" << url;
        return Mixxx::SoundSourceProviderPointer();
    }

    auto pSoundSourceProvider(
            s_soundSourceProviders.getProviderForFileExtension(
                    fileExtension));
    if (!pSoundSourceProvider) {
        qWarning() << "Unsupported file type:" << url;
    }

    return pSoundSourceProvider;
}

//static
Result SoundSourceProxy::saveTrackMetadata(const TrackInfoObject* pTrack) {
    SoundSourceProxy proxy(pTrack);
    Mixxx::TrackMetadata trackMetadata;
    bool parsedFromFile;
    pTrack->getTrackMetadata(&trackMetadata, &parsedFromFile);
    return proxy.writeTrackMetadata(trackMetadata);
}

SoundSourceProxy::SoundSourceProxy(const TrackPointer& pTrack)
    : m_pTrack(pTrack),
      m_filePath(pTrack->getCanonicalLocation()),
      m_url(QUrl::fromLocalFile(m_filePath)),
      m_pSoundSourceProvider(findSoundSourceProvider(m_url)) {
    initSoundSource();
}

SoundSourceProxy::SoundSourceProxy(const TrackInfoObject* pTrack)
    : m_filePath(pTrack->getCanonicalLocation()),
      m_url(QUrl::fromLocalFile(m_filePath)),
      m_pSoundSourceProvider(findSoundSourceProvider(m_url)) {
    initSoundSource();
}

Mixxx::SoundSourceProviderPointer SoundSourceProxy::getSoundSourceProvider() const {
    return m_pSoundSourceProvider;
}

void SoundSourceProxy::initSoundSource() {
    DEBUG_ASSERT(m_pSoundSource.isNull());
    DEBUG_ASSERT(m_pAudioSource.isNull());
    while (m_pSoundSource.isNull()) {
        Mixxx::SoundSourceProviderPointer pProvider(getSoundSourceProvider());
        if (pProvider.isNull()) {
            qWarning() << "Failed to obtain SoundSource for file"
                    << getFilePath();
            return; // failure -> exit loop
        }
        m_pSoundSource = pProvider->newSoundSource(m_url);
        if (m_pSoundSource.isNull()) {
            qWarning() << "Failed to obtain SoundSource for file"
                    << getFilePath()
                    << "from provider"
                    << pProvider->getName();
        } else {
            qDebug() << "Obtained SoundSource for"
                    << getFilePath()
                    << "from provider"
                    << pProvider->getName();
            return; // success -> exit loop
        }
    }
}

namespace {

// Keeps the TIO alive while accessing the audio data
// of the track. The TIO must not be deleted while
// accessing the corresponding file to avoid file
// corruption when writing metadata while the file
// is still in use.
class AudioSourceProxy: public Mixxx::AudioSource {
public:
    AudioSourceProxy(const AudioSourceProxy&) = delete;
    AudioSourceProxy(AudioSourceProxy&&) = delete;

    static Mixxx::AudioSourcePointer create(
            const TrackPointer& pTrack,
            const Mixxx::AudioSourcePointer& pAudioSource) {
        DEBUG_ASSERT(!pTrack.isNull());
        DEBUG_ASSERT(!pAudioSource.isNull());
        return Mixxx::AudioSourcePointer(
                new AudioSourceProxy(pTrack, pAudioSource));
    }

    AudioSourceProxy(
            const TrackPointer& pTrack,
            const Mixxx::AudioSourcePointer& pAudioSource)
        : Mixxx::AudioSource(*pAudioSource),
          m_pTrack(std::move(pTrack)),
          m_pAudioSource(std::move(pAudioSource)) {
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
    const Mixxx::AudioSourcePointer m_pAudioSource;
};

} // anonymous namespace

Mixxx::AudioSourcePointer SoundSourceProxy::openAudioSource(const Mixxx::AudioSourceConfig& audioSrcCfg) {
    while (m_pAudioSource.isNull()) {
        if (m_pSoundSource.isNull()) {
            qWarning() << "Failed to open AudioSource for file"
                    << getFilePath();
            return m_pAudioSource; // failure -> exit loop
        }
        if (OK == m_pSoundSource->open(audioSrcCfg)) {
            qDebug() << "Opened AudioSource for file"
                    << getFilePath()
                    << "with provider"
                    << getSoundSourceProvider()->getName();
            if (m_pSoundSource->isValid()) {
                m_pAudioSource =
                        AudioSourceProxy::create(
                                m_pTrack, m_pSoundSource);
                if (m_pAudioSource->isEmpty()) {
                    qWarning() << "Empty audio data in file"
                            << getFilePath();
                }
                // Overwrite metadata with actual audio properties
                if (m_pTrack) {
                    m_pTrack->setChannels(m_pAudioSource->getChannelCount());
                    m_pTrack->setSampleRate(m_pAudioSource->getFrameRate());
                    if (m_pAudioSource->hasDuration()) {
                        m_pTrack->setDuration(m_pAudioSource->getDuration());
                    }
                    if (m_pAudioSource->hasBitrate()) {
                        m_pTrack->setBitrate(m_pAudioSource->getBitrate());
                    }
                }
                return m_pAudioSource; // success -> exit loop
            } else {
                qWarning() << "Invalid audio data in file"
                        << getFilePath();
                // Do NOT retry with the next SoundSource provider if
                // only the file itself is malformed!
                m_pSoundSource->close();
                break; // exit loop
            }
        }
        qWarning() << "Failed to open AudioSource for file"
                << getFilePath()
                << "with provider"
                << getSoundSourceProvider()->getName();
    }
    // m_pSoundSource might be invalid when reaching this point
    qWarning() << "Failed to open AudioSource for file"
            << getFilePath();
    return m_pAudioSource;
}

void SoundSourceProxy::closeAudioSource() {
    if (!m_pAudioSource.isNull()) {
        DEBUG_ASSERT(!m_pSoundSource.isNull());
        m_pSoundSource->close();
        m_pAudioSource.clear();
        qDebug() << "Closed AudioSource for file"
                << getFilePath();
    }
}
