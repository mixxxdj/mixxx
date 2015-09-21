#ifndef SOUNDSOURCEPROXY_H
#define SOUNDSOURCEPROXY_H

#include "trackinfoobject.h"

#include "sources/soundsourceproviderregistry.h"

#include "util/sandbox.h"

// Creates sound sources for plain files or tracks. Only intended to be used
// in a narrow scope and not shareable between multiple threads!
class SoundSourceProxy: public Mixxx::MetadataSource {
public:
    static void loadPlugins(); // not thread-safe

    static QStringList getSupportedFileExtensions();
    static QStringList getSupportedFileExtensionsByPlugins();
    static QStringList getSupportedFileNamePatterns();
    static QRegExp getSupportedFileNameRegex();

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

    static Mixxx::SoundSourcePointer initialize(const QString& qFilename);

    const TrackPointer m_pTrack;
    const SecurityTokenPointer m_pSecurityToken;

    const QString m_filePath;
    const QUrl m_url;

    // This pointer must stay in this class together with
    // the corresponding track pointer. Don't pass it around!!
    const Mixxx::SoundSourcePointer m_pSoundSource;

    // Keeps track of opening and closing the corresponding
    // SoundSource. This pointer can safely be passed around,
    // because internally it contains a reference to the TIO
    // that keeps it alive.
    Mixxx::AudioSourcePointer m_pAudioSource;
};

#endif // SOUNDSOURCEPROXY_H
