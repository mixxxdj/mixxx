#ifndef MIXXX_SOURCES_SOUNDSOURCEPROXY_H
#define MIXXX_SOURCES_SOUNDSOURCEPROXY_H

#include "track/track.h"

#include "sources/soundsourceproviderregistry.h"

// Creates sound sources for tracks. Only intended to be used
// in a narrow scope and not shareable between multiple threads!
class SoundSourceProxy {
  public:
    // Initially registers all built-in SoundSource providers. This function is
    // not thread-safe and must be called only once upon startup of the
    // application.
    static void registerSoundSourceProviders();

    static QStringList getSupportedFileExtensions() {
        return s_soundSourceProviders.getRegisteredFileExtensions();
    }
    static const QStringList& getSupportedFileNamePatterns() {
        return s_supportedFileNamePatterns;
    }
    static const QRegExp& getSupportedFileNamesRegex() {
        return s_supportedFileNamesRegex;
    }

    static bool isUrlSupported(const QUrl& url);
    static bool isFileSupported(const TrackFile& trackFile);
    static bool isFileSupported(const QFileInfo& fileInfo);
    static bool isFileNameSupported(const QString& fileName);
    static bool isFileExtensionSupported(const QString& fileExtension);

    // The following import functions ensure that the file will not be
    // written while reading it!
    static TrackPointer importTemporaryTrack(
            TrackFile trackFile,
            SecurityTokenPointer pSecurityToken = SecurityTokenPointer());
    static QImage importTemporaryCoverImage(
            TrackFile trackFile,
            SecurityTokenPointer pSecurityToken = SecurityTokenPointer());

    explicit SoundSourceProxy(
            TrackPointer pTrack);

    const TrackPointer& getTrack() const {
        return m_pTrack;
    }

    mixxx::SoundSourceProviderPointer getSoundSourceProvider() const;

    const QUrl& getUrl() const {
        return m_url;
    }

    // Controls which (metadata/coverart) and how tags are (re-)imported from
    // audio files when creating a SoundSourceProxy.
    enum class ImportTrackMetadataMode {
        // Import both track metadata and cover image once for new track objects.
        // Otherwise the request is ignored and the track object is not modified.
        Once,
        // (Re-)Import the track's metadata and cover art. Cover art is
        // only updated if it has been guessed from metadata to prevent
        // overwriting a custom choice.
        Again,
        // If omitted both metadata and cover image will be imported at most
        // once for each track object to avoid overwriting modified data in
        // the library.
        Default = Once,
    };

    // Updates file type, metadata, and cover image of the track object
    // from the source file according to the given mode.
    //
    // The track's type will always be set as recognized by the corresponding
    // SoundSource.
    //
    // Importing of metadata and cover image is skipped if the track object
    // is marked as dirty or if mode=Once and the import has already been
    // performed once. Otherwise track metadata is set according to the metadata
    // imported from the file.
    //
    // An existing cover image is only replaced if it also has been imported
    // from the file. Custom cover images that have been selected by the user
    // are preserved.
    //
    // This function works in a best effort manner without returning a value.
    // Only the track object will be modified as a side effect. There are simply
    // too many possible reasons for failure to consider that cannot be handled
    // properly. The application log will contain warning messages for a detailed
    // analysis in case unexpected behavior has been reported.
    void updateTrackFromSource(
            ImportTrackMetadataMode importTrackMetadataMode = ImportTrackMetadataMode::Default) const;

    // Parse only the metadata from the file without modifying
    // the referenced track.
    mixxx::MetadataSource::ImportResult importTrackMetadata(mixxx::TrackMetadata* pTrackMetadata) const;

    // Opening the audio source through the proxy will update the
    // audio properties of the corresponding track object. Returns
    // a null pointer on failure.
    mixxx::AudioSourcePointer openAudioSource(
            const mixxx::AudioSource::OpenParams& params = mixxx::AudioSource::OpenParams());

    void closeAudioSource();

  private:
    static mixxx::SoundSourceProviderRegistry s_soundSourceProviders;
    static QStringList s_supportedFileNamePatterns;
    static QRegExp s_supportedFileNamesRegex;

    friend class TrackCollection;
    static Track::ExportMetadataResult exportTrackMetadataBeforeSaving(Track* pTrack);

    // Special case: Construction from a url is needed
    // for writing metadata immediately before the TIO is destroyed.
    explicit SoundSourceProxy(
            const QUrl& url);

    // Parse only the cover image from the file without modifying
    // the referenced track.
    QImage importCoverImage() const;

    const TrackPointer m_pTrack;

    const QUrl m_url;

    static QList<mixxx::SoundSourceProviderRegistration> findSoundSourceProviderRegistrations(const QUrl& url);

    const QList<mixxx::SoundSourceProviderRegistration> m_soundSourceProviderRegistrations;
    int m_soundSourceProviderRegistrationIndex;

    void nextSoundSourceProvider();

    void initSoundSource();

    // This pointer must stay in this class together with
    // the corresponding track pointer. Don't pass it around!!
    mixxx::SoundSourcePointer m_pSoundSource;

    // Keeps track of opening and closing the corresponding
    // SoundSource. This pointer can safely be passed around,
    // because internally it contains a reference to the TIO
    // that keeps it alive.
    mixxx::AudioSourcePointer m_pAudioSource;
};

#endif // MIXXX_SOURCES_SOUNDSOURCEPROXY_H
