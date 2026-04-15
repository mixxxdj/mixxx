#pragma once

#include <gtest/gtest_prod.h>

#include <QMimeType>

#include "sources/soundsourceproviderregistry.h"
#include "track/track_decl.h"

namespace mixxx {

class FileAccess;
class FileInfo;

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

    static QStringList getSupportedFileTypes() {
        return s_soundSourceProviders.getRegisteredFileTypes();
    }
    static const QStringList& getSupportedFileNamePatterns() {
        return s_supportedFileNamePatterns;
    }
    static const QRegularExpression& getSupportedFileNamesRegex() {
        return s_supportedFileNamesRegex;
    }
    static QString getFileTypeByMimeType(const QMimeType& mimeType) {
        return s_fileTypeByMimeType.value(mimeType);
    }

    /// Get the list of supported file extensions
    ///
    /// A single file type may map to multiple file suffixes, e.g.
    /// "aiff" to "aif" or "aiff".
    static QStringList getSupportedFileSuffixes();

    static QStringList getFileSuffixesForFileType(const QString& fileType);

    static bool isUrlSupported(const QUrl& url);
    static bool isFileSupported(const mixxx::FileInfo& fileInfo);
    static bool isFileNameSupported(const QString& fileName);
    static bool isFileTypeSupported(const QString& fileType);
    static bool isFileSuffixSupported(const QString& fileSuffix);

    static QList<mixxx::SoundSourceProviderRegistration> allProviderRegistrationsForUrl(
            const QUrl& url);
    static QList<mixxx::SoundSourceProviderRegistration> allProviderRegistrationsForFileType(
            const QString& fileType) {
        return s_soundSourceProviders.getRegistrationsForFileType(fileType);
    }
    static mixxx::SoundSourceProviderPointer getPrimaryProviderForFileType(
            const QString& fileType);

    explicit SoundSourceProxy(TrackPointer pTrack);

    // Only needed for testing all available providers explicitly
    SoundSourceProxy(
            TrackPointer pTrack,
            mixxx::SoundSourceProviderPointer pProvider);

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
            const mixxx::FileAccess& trackFileAccess,
            mixxx::TrackMetadata* pTrackMetadata,
            QImage* pCoverImage,
            bool resetMissingTagMetadata);

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
            QImage* pCoverImage,
            bool resetMissingTagMetadata) const;

    /// Controls which (metadata/coverart) and how tags are (re-)imported from
    /// audio files when creating a SoundSourceProxy.
    ///
    /// Cover art is only re-imported and updated if it has been guessed from
    /// metadata to prevent overwriting a custom choice.
    enum class UpdateTrackFromSourceMode {
        // Import both track metadata and cover image once for new track objects.
        // Otherwise the request is ignored and the track object is not modified.
        Once,
        /// (Re-)Import the track's metadata and cover art if the file's modification
        /// time stamp is newer than the last synchronization time stamp.
        ///
        /// Source synchronization time stamps have been introduced by v2.4.0.
        /// For existing files in the library this time stamp is undefined until
        /// metadata is manually re-imported! In this case we cannot determine
        /// if the file tags contain updated data and need to skip the implicit
        /// re-import to prevent overwriting precious user data.
        Newer,
        // Unconditionally (re-)import the track's metadata and cover art, independent
        // of when the file has last been modified and the synchronization time stamp.
        Always,
    };

    enum class UpdateTrackFromSourceResult {
        NotUpdated,
        MetadataImportFailed,
        MetadataImportedAndUpdated,
        ExtraMetadataImportedAndMerged,
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
    UpdateTrackFromSourceResult updateTrackFromSource(
            UpdateTrackFromSourceMode mode,
            const SyncTrackMetadataParams& syncParams);

    /// Opening the audio source through the proxy will update the
    /// audio properties of the corresponding track object. Returns
    /// a null pointer on failure.
    ///
    /// The caller is responsible for invoking AudioSource::close().
    /// Otherwise the underlying files will remain open until the
    /// last reference is dropped. One of these references is hold
    /// by SoundSourceProxy as a member.
    ///
    /// Note: If opening the audio stream fails the selection
    /// process may continue among the available providers and
    /// sound sources might be resumed and continue until a
    /// usable provider that could open the stream has been
    /// found.
    mixxx::AudioSourcePointer openAudioSource(
            const mixxx::AudioSource::OpenParams& params = mixxx::AudioSource::OpenParams());

  private:
    static mixxx::SoundSourceProviderRegistry s_soundSourceProviders;
    static QStringList s_supportedFileNamePatterns;
    static QRegularExpression s_supportedFileNamesRegex;
    static QHash<QMimeType, QString> s_fileTypeByMimeType;

    friend class TrackCollectionManager;
    FRIEND_TEST(TrackMetadataExportTest, keepWithespaceKey);
    static ExportTrackMetadataResult exportTrackMetadataBeforeSaving(
            Track* pTrack,
            const SyncTrackMetadataParams& syncParams);

    // Special case: Construction from a url is needed
    // for writing metadata immediately before the TIO is destroyed.
    explicit SoundSourceProxy(const QUrl& url);

    bool openSoundSource(
            const mixxx::AudioSource::OpenParams& params = mixxx::AudioSource::OpenParams());

    const TrackPointer m_pTrack;

    const QUrl m_url;

    const QList<mixxx::SoundSourceProviderRegistration> m_providerRegistrations;

    // Keeps track of the provider selection when creating the proxy
    // and while trying to open audio files. Starts at 0 for the primary
    // provider and is initialized with -1 if no
    int m_providerRegistrationIndex;

    void findProviderAndInitSoundSource();

    bool initSoundSourceWithProvider(
            mixxx::SoundSourceProviderPointer&& pProvider);

    mixxx::SoundSourceProviderPointer primaryProvider();
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
};
