#ifndef SOUNDSOURCEPROXY_H
#define SOUNDSOURCEPROXY_H

#include "trackinfoobject.h"

#include "sources/soundsourceproviderregistry.h"

// Creates sound sources for tracks. Only intended to be used
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

    // Load track metadata and (optionally) cover art from the file
    // if it has not already been parsed. With reloadFromFile = true
    // metadata and cover art will be reloaded from the file regardless
    // if it has already been parsed before or not.
    void loadTrackMetadata(bool reloadFromFile = false) const {
        return m_pTrack->parseTrackMetadata(*this, false, reloadFromFile);
    }
    void loadTrackMetadataAndCoverArt(bool reloadFromFile = false) const {
        return m_pTrack->parseTrackMetadata(*this, true, reloadFromFile);
    }

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

    // Opening the audio data through the proxy will
    // update the some metadata of the track object.
    // Returns a null pointer on failure.
    Mixxx::AudioSourcePointer openAudioSource(const Mixxx::AudioSourceConfig& audioSrcCfg = Mixxx::AudioSourceConfig());

    void closeAudioSource();

private:
    static Mixxx::SoundSourceProviderRegistry s_soundSourceProviders;
    static QStringList s_supportedFileNamePatterns;
    static QRegExp s_supportedFileNamesRegex;

    const TrackPointer m_pTrack;

    const QString m_filePath;
    const QUrl m_url;

    static QList<Mixxx::SoundSourceProviderRegistration> findSoundSourceProviderRegistrations(const QUrl& url);

    const QList<Mixxx::SoundSourceProviderRegistration> m_soundSourceProviderRegistrations;
    int m_soundSourceProviderRegistrationIndex;

    Mixxx::SoundSourceProviderPointer getSoundSourceProvider() const;
    void nextSoundSourceProvider();

    void initSoundSource();

    Mixxx::SoundSourcePointer m_pSoundSource;

    // Just an alias that keeps track of opening and closing
    // the corresponding SoundSource.
    Mixxx::AudioSourcePointer m_pAudioSource;
};

#endif // SOUNDSOURCEPROXY_H
