#ifndef SOUNDSOURCEPROXY_H
#define SOUNDSOURCEPROXY_H

#include "trackinfoobject.h"

#include "sources/soundsourceproviderregistry.h"

// Creates sound sources for plain files or tracks. Only intended to be used
// in a narrow scope and not shareable between multiple threads!
class SoundSourceProxy: public Mixxx::MetadataSource {
public:
    // Initially registers all built-in SoundSource providers and
    // loads all SoundSource plugins with additional providers. This
    // function is not thread-safe and must be called only once
    // upon startup of the application.
    static void loadPlugins();

    static QStringList getSupportedFileExtensions() {
        return s_soundSourceProviders.getRegisteredFileExtensions();
    }
    static QStringList getSupportedFileExtensionsByPlugins();
    static const QStringList& getSupportedFileNamePatterns() {
        return s_supportedFileNamePatterns;
    }
    static const QRegExp& getSupportedFileNamesRegex() {
        return s_supportedFileNamesRegex;
    }

    static bool isUrlSupported(const QUrl& url);
    static bool isFileSupported(const QFileInfo& fileInfo);
    static bool isFileNameSupported(const QString& fileName);
    static bool isFileExtensionSupported(const QString& fileExtension);

    explicit SoundSourceProxy(
            const TrackPointer& pTrack);

    const TrackPointer& getTrack() const {
        return m_pTrack;
    }

    const QString& getFilePath() const {
        return m_filePath;
    }

    const QUrl& getUrl() const {
        return m_url;
    }

    QString getType() const {
        if (m_pSoundSource) {
            return m_pSoundSource->getType();
        } else {
            return QString();
        }
    }

    // Load track metadata and (optionally) cover art from the file
    // if it has not already been parsed. With reloadFromFile = true
    // metadata and cover art will be reloaded from the file regardless
    // if it has already been parsed before or not.
    void loadTrackMetadata(bool reloadFromFile = false) const {
        m_pTrack->parseTrackMetadata(*this, false, reloadFromFile);
    }
    void loadTrackMetadataAndCoverArt(bool reloadFromFile = false) const {
        m_pTrack->parseTrackMetadata(*this, true, reloadFromFile);
    }
    static Result saveTrackMetadata(const TrackInfoObject* pTrack);

    // Low-level function for parsing track metadata and cover art
    // embedded in the audio file into the corresponding objects.
    // The track referenced by this proxy is not modified! Both
    // parameters are optional and can be set to nullptr/NULL.
    Result parseTrackMetadataAndCoverArt(
            Mixxx::TrackMetadata* pTrackMetadata,
            QImage* pCoverArt) const override {
        if (m_pSoundSource) {
            return m_pSoundSource->parseTrackMetadataAndCoverArt(
                    pTrackMetadata, pCoverArt);
        } else {
            return ERR;
        }
    }
    // Low-level function for writing track metadata back into
    // the audio file.
    Result writeTrackMetadata(
            const Mixxx::TrackMetadata& trackMetadata) const override {
        if (m_pSoundSource) {
            return m_pSoundSource->writeTrackMetadata(
                    trackMetadata);
        } else {
            return ERR;
        }
    }

    // Opening the audio data through the proxy will
    // update the some metadata of the track object.
    // Returns a null pointer on failure.
    Mixxx::AudioSourcePointer openAudioSource(const Mixxx::AudioSourceConfig& audioSrcCfg = Mixxx::AudioSourceConfig());

    void closeAudioSource();

private:
    static Mixxx::SoundSourceProviderRegistry s_soundSourceProviders;
    static QStringList s_supportedFileNamePatterns;
    static QRegExp s_supportedFileNamesRegex;

    // Special case: Construction from a plain TIO pointer is needed
    // for writing metadata immediately before the TIO is destroyed.
    explicit SoundSourceProxy(
            const TrackInfoObject* pTrack);

    const TrackPointer m_pTrack;

    const QString m_filePath;
    const QUrl m_url;

    static Mixxx::SoundSourceProviderPointer findSoundSourceProvider(
            const QUrl& url);

    const Mixxx::SoundSourceProviderPointer m_pSoundSourceProvider;

    Mixxx::SoundSourceProviderPointer getSoundSourceProvider() const;

    void initSoundSource();

    // This pointer must stay in this class together with
    // the corresponding track pointer. Don't pass it around!!
    Mixxx::SoundSourcePointer m_pSoundSource;

    // Keeps track of opening and closing the corresponding
    // SoundSource. This pointer can safely be passed around,
    // because internally it contains a reference to the TIO
    // that keeps it alive.
    Mixxx::AudioSourcePointer m_pAudioSource;
};

#endif // SOUNDSOURCEPROXY_H
