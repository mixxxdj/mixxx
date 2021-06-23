#pragma once

#include "preferences/usersettings.h"
#include "sources/soundsourceproviderregistry.h"
#include "track/track_decl.h"
#include "util/sandbox.h"

namespace mixxx {

class FileAccess;

} // namespace mixxx

/// Creates sound sources for tracks. Only intended to be used
/// in a narrow scope and not shareable between multiple threads!
class SoundSourceProxy {
  public:
    /// Initially registers all built-in SoundSource providers. This function is
    /// not thread-safe and must be called only once upon startup of the
    /// application.
    ///
    /// Returns true if providers for one or more file extensions have been
    /// registered.
    static bool registerProviders();

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
    static bool isFileSupported(const mixxx::FileInfo& fileInfo);
    static bool isFileNameSupported(const QString& fileName);
    static bool isFileExtensionSupported(const QString& fileExtension);

    static QList<mixxx::SoundSourceProviderRegistration> allProviderRegistrationsForUrl(
            const QUrl& url);
    static QList<mixxx::SoundSourceProviderRegistration> allProviderRegistrationsForFileExtension(
            const QString& fileExtension) {
        return s_soundSourceProviders.getRegistrationsForFileExtension(fileExtension);
    }
    static mixxx::SoundSourceProviderPointer getPrimaryProviderForFileExtension(
            const QString& fileExtension);

    explicit SoundSourceProxy(
            TrackPointer pTrack,
            const mixxx::SoundSourceProviderPointer& pProvider = nullptr);

    /// The track object that has been passed at construction.
    ///
    /// Holding an internal pointer to the track object ensures
    /// that all write operations to this file are deferred until
    /// the track object is released and not accessed concurrently.
    const TrackPointer& getTrack() const {
        return m_pTrack;
    }

    /// The URL of the audio file referenced by the track.
    const QUrl& getUrl() const {
        return m_url;
    }

    /// The provider that is currently in use.
    ///
    /// Note: This might change later after construction when actually
    /// trying to read the audio stream with openAudioSource()!
    const mixxx::SoundSourceProviderPointer& getProvider() const {
        return m_pProvider;
    }

    /// Import both track metadata and/or cover image from a file.
    ///
    /// Pass nullptr for an out parameter if the corresponding data
    /// is not needed.
    ///
    /// This function is thread-safe and can be invoked from any thread.
    /// It ensures that no other thread writes the file concurrently
    /// by keeping the corresponding file location in GlobalTrackCache
    /// while reading.
    static std::pair<mixxx::MetadataSource::ImportResult, QDateTime>
    importTrackMetadataAndCoverImageFromFile(
            mixxx::FileAccess trackFileAccess,
            mixxx::TrackMetadata* pTrackMetadata,
            QImage* pCoverImage);

    /// Import both track metadata and/or the cover image of the
    /// captured track object from the corresponding file.
    ///
    /// The captured track object is not modified, i.e. the data is read
    /// from the file directly into the provided out parameters. Pass nullptr
    /// for an out parameter if the corresponding data is not needed.
    ///
    /// If the captured track pointer is managed by GlobalTrackCache
    /// reading from the file is safe, i.e. the read operation could
    /// not be interleaved with a write operation when exporting metadata.
    std::pair<mixxx::MetadataSource::ImportResult, QDateTime>
    importTrackMetadataAndCoverImage(
            mixxx::TrackMetadata* pTrackMetadata,
            QImage* pCoverImage) const;

    /// Controls which (metadata/coverart) and how tags are (re-)imported from
    /// audio files when creating a SoundSourceProxy.
    enum class UpdateTrackFromSourceMode {
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

    /// Updates file type, metadata, and cover image of the track object
    /// from the source file according to the given mode.
    ///
    /// The track's type will always be set as recognized by the corresponding
    /// SoundSource.
    ///
    /// Importing of metadata and cover image is skipped if the track object
    /// is marked as dirty or if mode=Once and the import has already been
    /// performed once. Otherwise track metadata is set according to the metadata
    /// imported from the file.
    ///
    /// An existing cover image is only replaced if it also has been imported
    /// from the file. Custom cover images that have been selected by the user
    /// are preserved.
    ///
    /// This function works in a best effort manner without returning a value.
    /// Only the track object will be modified as a side effect. There are simply
    /// too many possible reasons for failure to consider that cannot be handled
    /// properly. The application log will contain warning messages for a detailed
    /// analysis in case unexpected behavior has been reported.
    ///
    /// Returns true if the track has been modified and false otherwise.
    bool updateTrackFromSource(
            UpdateTrackFromSourceMode mode = UpdateTrackFromSourceMode::Default);

    /// Opening the audio source through the proxy will update the
    /// audio properties of the corresponding track object. Returns
    /// a null pointer on failure.
    ///
    /// Note: If opening the audio stream fails the selection
    /// process may continue among the available providers and
    /// sound sources might be resumed and continue until a
    /// usable provider that could open the stream has been
    /// found.
    mixxx::AudioSourcePointer openAudioSource(
            const mixxx::AudioSource::OpenParams& params = mixxx::AudioSource::OpenParams());

    /// Explicitly close the AudioSource.
    ///
    /// This will happen implicitly when the instance goes out
    /// of scope, i.e. upon destruction.
    void closeAudioSource();

  private:
    static mixxx::SoundSourceProviderRegistry s_soundSourceProviders;
    static QStringList s_supportedFileNamePatterns;
    static QRegExp s_supportedFileNamesRegex;

    friend class TrackCollectionManager;
    static ExportTrackMetadataResult exportTrackMetadataBeforeSaving(
            Track* pTrack,
            const UserSettingsPointer& pConfig);

    // Special case: Construction from a url is needed
    // for writing metadata immediately before the TIO is destroyed.
    explicit SoundSourceProxy(
            const QUrl& url,
            const mixxx::SoundSourceProviderPointer& pProvider = nullptr);

    const TrackPointer m_pTrack;

    const QUrl m_url;

    const QList<mixxx::SoundSourceProviderRegistration> m_providerRegistrations;

    // Keeps track of the provider selection when creating the proxy
    // and while trying to open audio files. Starts at 0 for the primary
    // provider and is initialized with -1 if no
    int m_providerRegistrationIndex;

    void initSoundSource(
            const mixxx::SoundSourceProviderPointer& pProvider);

    mixxx::SoundSourceProviderPointer primaryProvider(
            const mixxx::SoundSourceProviderPointer& pProvider = nullptr);
    mixxx::SoundSourceProviderPointer nextProvider();
    std::pair<mixxx::SoundSourceProviderPointer, mixxx::SoundSource::OpenMode>
            nextProviderWithOpenMode(mixxx::SoundSource::OpenMode);

    // The provider that has actually been used to create the
    // SoundSource (see below). Always set in conjunction with
    // the SoundSource.
    mixxx::SoundSourceProviderPointer m_pProvider;

    // This pointer must stay in this class together with
    // the corresponding track pointer. Don't pass it around!!
    mixxx::SoundSourcePointer m_pSoundSource;

    // Keeps track of opening and closing the corresponding
    // SoundSource. This pointer can safely be passed around,
    // because internally it contains a reference to the TIO
    // that keeps it alive.
    mixxx::AudioSourcePointer m_pAudioSource;
};
