#include "sources/soundsourceproxy.h"

#include <QMimeDatabase>
#include <QMimeType>
#include <QRegularExpression>
#include <QStandardPaths>

#include "sources/audiosourcetrackproxy.h"

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
#ifdef __FFMPEG__
#include "sources/soundsourceffmpeg.h"
#endif
#ifdef __MODPLUG__
#include "sources/soundsourcemodplug.h"
#endif
#ifdef __FAAD__
#include "sources/soundsourcem4a.h"
#endif
#ifdef __WV__
#include "sources/soundsourcewv.h"
#endif
#include "sources/soundsourceflac.h"
#ifdef __MEDIAFOUNDATION__
#include "sources/soundsourcemediafoundation.h"
#endif
#ifdef __STEM__
#include "sources/soundsourcestem.h"
#endif

#include "library/coverartutils.h"
#include "track/globaltrackcache.h"
#include "track/track.h"
#include "util/logger.h"
#include "util/regex.h"

//Static memory allocation
/*static*/ mixxx::SoundSourceProviderRegistry SoundSourceProxy::s_soundSourceProviders;
/*static*/ QStringList SoundSourceProxy::s_supportedFileNamePatterns;
/*static*/ QRegularExpression SoundSourceProxy::s_supportedFileNamesRegex;
/*static*/ QHash<QMimeType, QString> SoundSourceProxy::s_fileTypeByMimeType;

namespace {

const mixxx::Logger kLogger("SoundSourceProxy");

bool registerSoundSourceProvider(
        mixxx::SoundSourceProviderRegistry* pProviderRegistry,
        const mixxx::SoundSourceProviderPointer& pProvider) {
    VERIFY_OR_DEBUG_ASSERT(pProvider) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(
            pProviderRegistry->registerProvider(pProvider) > 0) {
        kLogger.warning()
                << "Failed to register SoundSource provider"
                << pProvider->getDisplayName();
        return false;
    }
    return true;
}

void registerReferenceSoundSourceProvider(
        mixxx::SoundSourceProviderRegistry* pProviderRegistry,
        const mixxx::SoundSourceProviderPointer& pProvider) {
    if (!registerSoundSourceProvider(pProviderRegistry, pProvider)) {
        return;
    }
    // Verify that the provider is the primary provider for all
    // supported file types
    const QStringList supportedFileTypes = pProvider->getSupportedFileTypes();
    for (const auto& fileType : supportedFileTypes) {
        const auto pPrimaryProvider =
                pProviderRegistry->getPrimaryProviderForFileType(fileType);
        VERIFY_OR_DEBUG_ASSERT(pPrimaryProvider == pProvider) {
            kLogger.warning()
                    << "Using SoundSource provider"
                    << pPrimaryProvider->getDisplayName()
                    << "instead of"
                    << pProvider->getDisplayName()
                    << "for file type"
                    << fileType;
        }
    }
}

/// Register the default audio decoder(s) for the platform.
///
/// The default providers are chosen with precedence if multiple
/// SoundSource providers are available for a given file type
/// and priority level. This is achieved by registering them before
/// all other providers.
bool registerPlatformAndFallbackSoundSourceProviders(
        mixxx::SoundSourceProviderRegistry* pProviderRegistry) {
    VERIFY_OR_DEBUG_ASSERT(pProviderRegistry) {
        return false;
    }
    kLogger.debug()
            << "Registering platform and fallback SoundSource providers";
#if defined(__COREAUDIO__)
    // Core Audio is exclusively available on macOS and mandatory
    // if enabled
    VERIFY_OR_DEBUG_ASSERT(
            registerSoundSourceProvider(
                    pProviderRegistry,
                    std::make_shared<mixxx::SoundSourceProviderCoreAudio>())) {
        return false;
    }
#endif // __COREAUDIO__
#if defined(__MEDIAFOUNDATION__)
    // Media Foundation is exclusively available on Windows and mandatory
    // if enabled
    VERIFY_OR_DEBUG_ASSERT(
            registerSoundSourceProvider(
                    pProviderRegistry,
                    std::make_shared<mixxx::SoundSourceProviderMediaFoundation>())) {
        return false;
    }
#endif // __MEDIAFOUNDATION__
#if defined(__FFMPEG__)
    // FFmpeg might be available on any platform, and is used both
    // as the common default and fallback provider.
    registerSoundSourceProvider(
            pProviderRegistry,
            std::make_shared<mixxx::SoundSourceProviderFFmpeg>());
#endif // __FFMPEG__
    return true;
}

/// Register the reference audio decoders for their respective formats.
///
/// These providers are always chosen with a higher priority and could
/// therefore be registered after all other providers.
void registerReferenceSoundSourceProviders(
        mixxx::SoundSourceProviderRegistry* pProviderRegistry) {
    VERIFY_OR_DEBUG_ASSERT(pProviderRegistry) {
        return;
    }
    kLogger.debug()
            << "Registering reference SoundSource providers";
    registerReferenceSoundSourceProvider(
            pProviderRegistry,
            std::make_shared<mixxx::SoundSourceProviderFLAC>());
    registerReferenceSoundSourceProvider(
            pProviderRegistry,
            std::make_shared<mixxx::SoundSourceProviderOggVorbis>());
#ifdef __OPUS__
    registerReferenceSoundSourceProvider(
            pProviderRegistry,
            std::make_shared<mixxx::SoundSourceProviderOpus>());
#endif
#ifdef __WV__
    registerReferenceSoundSourceProvider(
            pProviderRegistry,
            std::make_shared<mixxx::SoundSourceProviderWV>());
#endif
}

QList<QMimeType> mimeTypesForFileType(const QString& fileType) {
    const QString dummyFileName = QStringLiteral("prefix.") + fileType;
    auto mimeTypes = QMimeDatabase().mimeTypesForFileName(dummyFileName);
    if (fileType == "opus") {
        // *.opus suffix is registered for QMimeType("audio/ogg") and QMimeType("audio/x-opus+ogg")
        // the Mixxx fileType "opus" only supports QMimeType("audio/x-opus+ogg");
        for (auto& mimeType : mimeTypes) {
            if (mimeType.name() == "audio/x-opus+ogg") {
                return {mimeType};
            }
        }
    }
    if (fileType == "stem.mp4" || fileType == "stem.m4a") {
        // *.stem.mp4 and *.stem.m4a suffix do not have a specific MIME type
        // associated with them, and simply fall back to MP4 mime type. To
        // prevent conflicts with the MP4 decoder  which is already mapped for
        // the MP4 mime type, able to decode arbitrary MP4 file (such as video,
        // extracting just the audio track), we don't return any MIME here. In
        // the future, if NI STEM gets assigned a MIME, we should return it.
        return {};
    }
    return mimeTypes;
}

} // anonymous namespace

// static
bool SoundSourceProxy::registerProviders() {
    // Initialize built-in file types.
    // Fallback providers should be registered before specialized
    // providers to ensure that they are only after the specialized
    // provider failed to open a file. But the order of registration
    // only matters among providers with equal priority.
    kLogger.debug()
            << "Registering SoundSource providers";
    // Register the platform and fallback providers BEFORE all other
    // providers to prioritize them by their order of registration,
    // preceding any other provider that is registered with the same
    // priority later.
    VERIFY_OR_DEBUG_ASSERT(
            registerPlatformAndFallbackSoundSourceProviders(&s_soundSourceProviders)) {
        kLogger.warning()
                << "Failed to register platform and fallback SoundSource providers";
        // This must not fail
        return false;
    }
    // Register other SoundSource providers
#ifdef __MAD__
    registerSoundSourceProvider(
            &s_soundSourceProviders,
            std::make_shared<mixxx::SoundSourceProviderMp3>());
#endif
#ifdef __MODPLUG__
    registerSoundSourceProvider(
            &s_soundSourceProviders,
            std::make_shared<mixxx::SoundSourceProviderModPlug>());
#endif
#ifdef __FAAD__
    registerSoundSourceProvider(
            &s_soundSourceProviders,
            std::make_shared<mixxx::SoundSourceProviderM4A>());
#endif
#ifdef __SNDFILE__
    // libsndfile is another fallback
    registerSoundSourceProvider(
            &s_soundSourceProviders,
            std::make_shared<mixxx::SoundSourceProviderSndFile>());
#endif
#ifdef __STEM__
    registerSoundSourceProvider(
            &s_soundSourceProviders,
            std::make_shared<mixxx::SoundSourceProviderSTEM>());
#endif
    // Register the high-priority reference providers AFTER all other
    // providers to verify that their priorities are correct.
    registerReferenceSoundSourceProviders(&s_soundSourceProviders);

    const QStringList supportedFileTypes = getSupportedFileTypes();
    VERIFY_OR_DEBUG_ASSERT(!supportedFileTypes.isEmpty()) {
        kLogger.critical()
                << "No file types registered";
        return false;
    }

    for (const auto& supportedFileType : supportedFileTypes) {
        const auto mimeTypes = mimeTypesForFileType(supportedFileType);
        for (const QMimeType& mimeType : mimeTypes) {
            if (!mimeType.isDefault()) {
                qDebug() << mimeType << supportedFileType;
                DEBUG_ASSERT(s_fileTypeByMimeType.constFind(mimeType) ==
                        s_fileTypeByMimeType.constEnd());
                s_fileTypeByMimeType.insert(mimeType, supportedFileType);
            }
        }
        if (kLogger.infoEnabled()) {
            kLogger.info() << "SoundSource providers for file type" << supportedFileType;
            const QList<mixxx::SoundSourceProviderRegistration> registrationsForFileType(
                    s_soundSourceProviders.getRegistrationsForFileType(
                            supportedFileType));
            for (const auto& registration : registrationsForFileType) {
                kLogger.info()
                        << registration.getProviderPriority()
                        << ":"
                        << registration.getProvider()->getDisplayName();
            }
        }
    }

    // Turn the file suffix list into a [ "*.mp3", "*.wav", ... ] style string list
    const auto supportedFileSuffixes = getSupportedFileSuffixes();
    s_supportedFileNamePatterns.clear();
    s_supportedFileNamePatterns.reserve(supportedFileSuffixes.size());
    for (const auto& supportedFileSuffix : supportedFileSuffixes) {
        s_supportedFileNamePatterns.append(QStringLiteral("*.") + supportedFileSuffix);
    }

    // Build regular expression of supported file suffixes
    QString supportedFileNamesRegex =
            RegexUtils::fileExtensionsRegex(supportedFileSuffixes);
    s_supportedFileNamesRegex = QRegularExpression(
            supportedFileNamesRegex,
            QRegularExpression::CaseInsensitiveOption);

    return true;
}

// static
bool SoundSourceProxy::isUrlSupported(const QUrl& url) {
    return isFileSupported(mixxx::FileInfo::fromQUrl(url));
}

// static
bool SoundSourceProxy::isFileSupported(const mixxx::FileInfo& fileInfo) {
    const QString fileType =
            mixxx::SoundSource::getTypeFromFile(fileInfo.asQFileInfo());
    // qDebug() << "isFileSupported" << fileType;
    return isFileTypeSupported(fileType);
}

// static
bool SoundSourceProxy::isFileNameSupported(const QString& fileName) {
    return fileName.contains(getSupportedFileNamesRegex());
}

// static
bool SoundSourceProxy::isFileTypeSupported(const QString& fileType) {
    return !s_soundSourceProviders.getRegistrationsForFileType(fileType).isEmpty();
}

// static
bool SoundSourceProxy::isFileSuffixSupported(const QString& fileSuffix) {
    return getSupportedFileSuffixes().contains(fileSuffix);
}

//static
mixxx::SoundSourceProviderPointer SoundSourceProxy::getPrimaryProviderForFileType(
        const QString& fileType) {
    return s_soundSourceProviders.getPrimaryProviderForFileType(fileType);
}

//static
QStringList SoundSourceProxy::getFileSuffixesForFileType(
        const QString& fileType) {
    // Each file type is a valid file suffix
    const auto mimeTypes = mimeTypesForFileType(fileType);
    QStringList fileSuffixes;
    // Reserve some extra space to prevent allocations, assuming 2 suffixes per type on average
    fileSuffixes.reserve(mimeTypes.size() * 2);
    for (const QMimeType& mimeType : mimeTypes) {
        fileSuffixes.append(mimeType.suffixes());
    }
    fileSuffixes.append(fileType);
    fileSuffixes.removeDuplicates();
    return fileSuffixes;
}

//static
QStringList SoundSourceProxy::getSupportedFileSuffixes() {
    const auto fileTypes = getSupportedFileTypes();
    QStringList fileSuffixes;
    // Reserve some extra space to prevent allocations, assuming 2 suffixes per type on average
    fileSuffixes.reserve(fileTypes.size() * 2);
    for (const QString& fileType : fileTypes) {
        fileSuffixes.append(getFileSuffixesForFileType(fileType));
    }
    fileSuffixes.removeDuplicates();
    return fileSuffixes;
}

// static
QList<mixxx::SoundSourceProviderRegistration>
SoundSourceProxy::allProviderRegistrationsForUrl(
        const QUrl& url) {
    if (url.isEmpty()) {
        // silently ignore empty URLs
        return {};
    }
    const QString fileType =
            mixxx::SoundSource::getTypeFromUrl(url);
    if (fileType.isEmpty()) {
        kLogger.warning()
                << "Unknown file type:"
                << url.toString();
        return {};
    }
    const auto providerRegistrations =
            allProviderRegistrationsForFileType(
                    fileType);
    if (providerRegistrations.isEmpty()) {
        kLogger.warning()
                << "Unsupported file type:"
                << url.toString();
    }
    return providerRegistrations;
}

//static
ExportTrackMetadataResult
SoundSourceProxy::exportTrackMetadataBeforeSaving(
        Track* pTrack,
        const SyncTrackMetadataParams& syncParams) {
    DEBUG_ASSERT(pTrack);
    const auto fileInfo = pTrack->getFileInfo();
    mixxx::SoundSourcePointer pSoundSource;
    {
        auto proxy = SoundSourceProxy(fileInfo.toQUrl());
        // Ensure that the actual audio properties of the
        // stream are available before exporting metadata.
        // This might be needed for converting sample positions
        // to time positions and vice versa.
        if (!pTrack->hasStreamInfoFromSource()) {
            if (proxy.openSoundSource()) {
                pSoundSource = proxy.m_pSoundSource;
                pTrack->updateStreamInfoFromSource(
                        pSoundSource->getStreamInfo());
                DEBUG_ASSERT(pTrack->hasStreamInfoFromSource());
                // Make sure that the track file is closed after reading.
                // Otherwise writing of track metadata into file tags
                // might fail.
                pSoundSource->close();
            } else {
                kLogger.warning()
                        << "Failed to update stream info from audio "
                           "source before exporting metadata";
                return ExportTrackMetadataResult::Failed;
            }
        }
        pSoundSource = proxy.m_pSoundSource;
    }
    if (!pSoundSource) {
        kLogger.warning()
                << "Unable to export track metadata into file"
                << fileInfo;
        return ExportTrackMetadataResult::Failed;
    }
    return pTrack->exportMetadata(*pSoundSource, syncParams);
}

// Used during tests only
SoundSourceProxy::SoundSourceProxy(
        TrackPointer pTrack,
        mixxx::SoundSourceProviderPointer pProvider)
        : m_pTrack(std::move(pTrack)),
          m_url(m_pTrack ? m_pTrack->getFileInfo().toQUrl() : QUrl()),
          m_providerRegistrationIndex(-1) {
    initSoundSourceWithProvider(std::move(pProvider));
}

SoundSourceProxy::SoundSourceProxy(TrackPointer pTrack)
        : m_pTrack(std::move(pTrack)),
          m_url(m_pTrack ? m_pTrack->getFileInfo().toQUrl() : QUrl()),
          m_providerRegistrations(allProviderRegistrationsForUrl(m_url)) {
    findProviderAndInitSoundSource();
}

SoundSourceProxy::SoundSourceProxy(const QUrl& url)
        : m_url(url),
          m_providerRegistrations(allProviderRegistrationsForUrl(m_url)) {
    findProviderAndInitSoundSource();
}

mixxx::SoundSourceProviderPointer SoundSourceProxy::primaryProvider() {
    m_providerRegistrationIndex = 0;
    if (m_providerRegistrationIndex >= m_providerRegistrations.size()) {
        return nullptr;
    }
    return m_providerRegistrations[m_providerRegistrationIndex].getProvider();
}

mixxx::SoundSourceProviderPointer SoundSourceProxy::nextProvider() {
    VERIFY_OR_DEBUG_ASSERT(m_providerRegistrationIndex >= 0) {
        return nullptr;
    }
    if (m_providerRegistrationIndex >= m_providerRegistrations.size()) {
        return nullptr;
    }
    ++m_providerRegistrationIndex;
    if (m_providerRegistrationIndex >= m_providerRegistrations.size()) {
        return nullptr;
    }
    return m_providerRegistrations[m_providerRegistrationIndex].getProvider();
}

std::pair<mixxx::SoundSourceProviderPointer, mixxx::SoundSource::OpenMode>
SoundSourceProxy::nextProviderWithOpenMode(
        mixxx::SoundSource::OpenMode openMode) {
    if (m_providerRegistrationIndex < 0) {
        if (openMode == mixxx::SoundSource::OpenMode::Strict) {
            // try again using m_pProvider, but only once
            openMode = mixxx::SoundSource::OpenMode::Permissive;
        } else {
            // abort
            DEBUG_ASSERT(openMode == mixxx::SoundSource::OpenMode::Permissive);
            m_pProvider.reset();
        }
        return std::make_pair(m_pProvider, openMode);
    } else {
        m_pProvider.reset();
        auto pNextProvider = nextProvider();
        if (!pNextProvider) {
            if (openMode == mixxx::SoundSource::OpenMode::Strict) {
                // try again, i.e. start next round
                openMode = mixxx::SoundSource::OpenMode::Permissive;
                pNextProvider = primaryProvider();
            } else {
                // abort
                DEBUG_ASSERT(openMode == mixxx::SoundSource::OpenMode::Permissive);
            }
        }
        return std::make_pair(pNextProvider, openMode);
    }
}

void SoundSourceProxy::findProviderAndInitSoundSource() {
    DEBUG_ASSERT(!m_pProvider);
    DEBUG_ASSERT(!m_pSoundSource);
    for (m_providerRegistrationIndex = 0;
            m_providerRegistrationIndex < m_providerRegistrations.size();
            ++m_providerRegistrationIndex) {
        mixxx::SoundSourceProviderPointer pProvider =
                m_providerRegistrations[m_providerRegistrationIndex]
                        .getProvider();
        VERIFY_OR_DEBUG_ASSERT(pProvider) {
            continue;
        }
        if (initSoundSourceWithProvider(std::move(pProvider))) {
            return; // Success
        }
    }
    if (!getUrl().isEmpty()) {
        kLogger.warning()
                << "No SoundSourceProvider for file"
                << getUrl().toString(QUrl::PreferLocalFile);
    }
}

bool SoundSourceProxy::initSoundSourceWithProvider(
        mixxx::SoundSourceProviderPointer&& pProvider) {
    DEBUG_ASSERT(!m_pProvider);
    DEBUG_ASSERT(!m_pSoundSource);
    DEBUG_ASSERT(pProvider);
    m_pSoundSource = pProvider->newSoundSource(m_url);
    if (!m_pSoundSource) {
        kLogger.warning() << "SoundSourceProvider"
                          << pProvider->getDisplayName()
                          << "failed to create a SoundSource for file"
                          << getUrl().toString(QUrl::PreferLocalFile);
        return false;
    }
    m_pProvider = pProvider;
    if (kLogger.debugEnabled()) {
        kLogger.debug() << "SoundSourceProvider"
                        << m_pProvider->getDisplayName()
                        << "created a SoundSource for file"
                        << getUrl().toString(QUrl::PreferLocalFile)
                        << "of type"
                        << m_pSoundSource->getType();
    }
    return true;
}

namespace {

inline std::pair<mixxx::MetadataSource::ImportResult, QDateTime>
importTrackMetadataAndCoverImageUnavailable() {
    return std::make_pair(mixxx::MetadataSource::ImportResult::Unavailable, QDateTime());
}

} // anonymous namespace

//static
std::pair<mixxx::MetadataSource::ImportResult, QDateTime>
SoundSourceProxy::importTrackMetadataAndCoverImageFromFile(
        const mixxx::FileAccess& trackFileAccess,
        mixxx::TrackMetadata* pTrackMetadata,
        QImage* pCoverImage,
        bool resetMissingTagMetadata) {
    if (!trackFileAccess.info().checkFileExists()) {
        // Silently ignore missing files to avoid spaming the log:
        // https://github.com/mixxxdj/mixxx/issues/9944
        return importTrackMetadataAndCoverImageUnavailable();
    }

    // Since the Track object below is only used to read the meta data from
    // the file, we use a temporary track that is discarded in an incomplete
    // state without being saved to the database or the track file's meta data
    // after the meta data import has been completed.
    // Lock the track via the global track cache while accessing the file to
    // ensure that no metadata is written by another thread.
    { // cacheResolver scope
        const bool temporary = true;
        auto cacheResolver = GlobalTrackCacheResolver(trackFileAccess, temporary);
        TrackPointer pTrack = cacheResolver.getTrack();
        // In the unlikely case that the file has disappeart since the check above
        // pTrack is null and the next call will log appropriate warnings and
        // returns importTrackMetadataAndCoverImageUnavailable();
        return SoundSourceProxy(pTrack).importTrackMetadataAndCoverImage(
                pTrackMetadata,
                pCoverImage,
                resetMissingTagMetadata);
    }
}

std::pair<mixxx::MetadataSource::ImportResult, QDateTime>
SoundSourceProxy::importTrackMetadataAndCoverImage(
        mixxx::TrackMetadata* pTrackMetadata,
        QImage* pCoverImage,
        bool resetMissingTagMetadata) const {
    if (!m_pSoundSource) {
        // The file doesn't seem to be readable or the file format
        // is not supported.
        return importTrackMetadataAndCoverImageUnavailable();
    }
    return m_pSoundSource->importTrackMetadataAndCoverImage(
            pTrackMetadata,
            pCoverImage,
            resetMissingTagMetadata);
}

namespace {

inline bool shouldUpdateTrackMetadataFromSource(
        mixxx::TrackRecord::SourceSyncStatus sourceSyncStatus,
        SoundSourceProxy::UpdateTrackFromSourceMode mode) {
    return mode == SoundSourceProxy::UpdateTrackFromSourceMode::Always ||
            (mode == SoundSourceProxy::UpdateTrackFromSourceMode::Newer &&
                    sourceSyncStatus == mixxx::TrackRecord::SourceSyncStatus::Outdated) ||
            (mode == SoundSourceProxy::UpdateTrackFromSourceMode::Once &&
                    sourceSyncStatus == mixxx::TrackRecord::SourceSyncStatus::Void);
}

inline bool shouldImportSeratoTagsFromSource(
        mixxx::TrackRecord::SourceSyncStatus sourceSyncStatus,
        const SyncTrackMetadataParams& syncParams) {
    // Only reimport track metadata from Serato markers if export of
    // Serato markers is enabled. This should ensure that track color,
    // cue points, and loops do not get lost after they have been
    // modified in Mixxx.
    // A reimport of metadata happens if
    // sourceSyncStatus == mixxx::TrackRecord::SourceSyncStatus::Outdated
    return sourceSyncStatus != mixxx::TrackRecord::SourceSyncStatus::Outdated ||
            syncParams.syncSeratoMetadata;
}

} // namespace

SoundSourceProxy::UpdateTrackFromSourceResult SoundSourceProxy::updateTrackFromSource(
        UpdateTrackFromSourceMode mode,
        const SyncTrackMetadataParams& syncParams) {
    DEBUG_ASSERT(m_pTrack);

    if (getUrl().isEmpty()) {
        // Silently skip tracks without a corresponding file
        return UpdateTrackFromSourceResult::NotUpdated;
    }
    if (!m_pSoundSource) {
        kLogger.warning()
                << "Unable to update track from unsupported file type"
                << getUrl().toString(QUrl::PreferLocalFile);
        return UpdateTrackFromSourceResult::NotUpdated;
    }

    // The SoundSource provides the actual type of the corresponding file
    const QString newType = m_pSoundSource->getType();
    DEBUG_ASSERT(!newType.isEmpty());
    const QString oldType = m_pTrack->setType(newType);
    if (!oldType.isEmpty() && oldType != newType) {
        // This should only happen for files with a wrong file extension
        // that have been added by version 2.3 or earlier.
        kLogger.warning() << "File type updated from"
                          << oldType
                          << "to"
                          << newType
                          << "for file"
                          << getUrl().toString(QUrl::PreferLocalFile);
    }

    // Use the existing track metadata as default values. Otherwise
    // existing values in the library would be overwritten with empty
    // values if the corresponding file tags are missing. Depending
    // on the file type some kind of tags might even not be supported
    // at all and this information would get lost entirely otherwise!
    mixxx::TrackRecord::SourceSyncStatus sourceSyncStatus;
    mixxx::TrackMetadata trackMetadata =
            m_pTrack->getMetadata(&sourceSyncStatus);

    if (sourceSyncStatus == mixxx::TrackRecord::SourceSyncStatus::Undefined) {
        kLogger.warning()
                << "Unable to update track from missing or inaccessible file"
                << getUrl().toString();
        return UpdateTrackFromSourceResult::NotUpdated;
    }

    // Save for later to replace the unreliable and imprecise audio
    // properties imported from file tags (see below).
    const auto preciseStreamInfo = trackMetadata.getStreamInfo();

    // Embedded cover art is imported together with the track's metadata.
    // But only if the user has not selected external cover art for this
    // track!
    QImage coverImg;
    DEBUG_ASSERT(coverImg.isNull());
    QImage* pCoverImg = nullptr; // pointer also serves as a flag

    const bool updateMetadataFromSource =
            shouldUpdateTrackMetadataFromSource(sourceSyncStatus, mode);

    // Decide if cover art needs to be re-imported
    if (updateMetadataFromSource) {
        const auto coverInfo = m_pTrack->getCoverInfo();
        // Avoid replacing user selected cover art with guessed cover art!
        if (coverInfo.source == CoverInfo::USER_SELECTED &&
                coverInfo.type == CoverInfo::FILE) {
            // Ignore embedded cover art
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Skip importing of embedded cover art from file"
                        << getUrl().toString(QUrl::PreferLocalFile);
            }
        } else {
            // Request reimport of embedded cover art
            pCoverImg = &coverImg;
        }
    }

    // Parse the tags stored in the audio file and the date and time when the
    // file has been last modified to detect future changes of the tags.
    auto [metadataImportResult, sourceSynchronizedAt] =
            importTrackMetadataAndCoverImage(
                    &trackMetadata,
                    pCoverImg,
                    syncParams.resetMissingTagMetadataOnImport);
    VERIFY_OR_DEBUG_ASSERT(!sourceSynchronizedAt.isValid() ||
            sourceSynchronizedAt.timeSpec() == Qt::UTC) {
        qWarning() << "Converting source synchronization time to UTC:" << sourceSynchronizedAt;
        sourceSynchronizedAt = sourceSynchronizedAt.toUTC();
    }
    if (metadataImportResult ==
            mixxx::MetadataSource::ImportResult::Failed) {
        kLogger.warning()
                << "Failed to import track metadata"
                << (pCoverImg ? "and embedded cover art" : "")
                << "from file"
                << getUrl().toString(QUrl::PreferLocalFile);
        // make sure that the trackMetadata was not messed up due to the failure
        mixxx::TrackRecord::SourceSyncStatus sourceSyncStatusNew;
        trackMetadata = m_pTrack->getMetadata(&sourceSyncStatusNew);
        if (sourceSyncStatus != sourceSyncStatusNew) {
            // Do not continue after detecting an inconsistency due to
            // race conditions while restoring the track metadata.
            // This is almost impossible, but it may happen. The preceding
            // warning message already identifies the track that is affected.
            kLogger.critical() << "Aborting update of track metadata from source "
                                  "due to unexpected inconsistencies during recovery";
            return UpdateTrackFromSourceResult::MetadataImportFailed;
        }
    }

    // Partial import
    if (!updateMetadataFromSource) {
        // No reimport of embedded cover image desired in this case.
        // Only import and merge extra metadata that might be missing
        // in the database.
        DEBUG_ASSERT(!pCoverImg);
        if (metadataImportResult == mixxx::MetadataSource::ImportResult::Succeeded) {
            if (m_pTrack->mergeExtraMetadataFromSource(trackMetadata)) {
                return UpdateTrackFromSourceResult::ExtraMetadataImportedAndMerged;
            } else {
                return UpdateTrackFromSourceResult::NotUpdated;
            }
        } else {
            // Nothing to do if no metadata has been imported
            return UpdateTrackFromSourceResult::NotUpdated;
        }
    }

    // Full import
    DEBUG_ASSERT(updateMetadataFromSource);
    if (!shouldImportSeratoTagsFromSource(sourceSyncStatus, syncParams)) {
        // Reset Serato tags to disable the (re-)import
        trackMetadata.refTrackInfo().refSeratoTags() = {};
    }
    if (sourceSyncStatus == mixxx::TrackRecord::SourceSyncStatus::Void) {
        DEBUG_ASSERT(pCoverImg);
        if (kLogger.debugEnabled()) {
            kLogger.debug()
                    << "Initializing track metadata and embedded cover art from file"
                    << getUrl().toString(QUrl::PreferLocalFile);
        }
    } else {
        if (kLogger.debugEnabled()) {
            kLogger.debug()
                    << "Re-importing track metadata"
                    << (pCoverImg ? "and embedded cover art" : "")
                    << "from file"
                    << getUrl().toString(QUrl::PreferLocalFile);
        }
    }

    // Preserve the precise stream info data (if available) that has been
    // obtained from the actual audio stream. If the file content itself
    // has been modified the stream info data will be updated next time
    // when opening and decoding the audio stream.
    if (preciseStreamInfo.isValid()) {
        trackMetadata.setStreamInfo(preciseStreamInfo);
    } else if (preciseStreamInfo.getSignalInfo().isValid()) {
        // Special case: Only the bitrate might be invalid or unknown
        trackMetadata.refStreamInfo().setSignalInfo(
                preciseStreamInfo.getSignalInfo());
        if (preciseStreamInfo.getDuration() > mixxx::Duration::empty()) {
            trackMetadata.refStreamInfo().setDuration(preciseStreamInfo.getDuration());
        }
    }

    // Ensure that all tracks have a title
    // Checking sourceSynchronizedAt.isValid() is required to skip inaccessible
    // or missing files!
    if (sourceSynchronizedAt.isValid() &&
            trackMetadata.getTrackInfo().getTitle().trimmed().isEmpty()) {
        // Only parse artist and title if both fields are empty to avoid
        // inconsistencies. Otherwise the file name (without extension)
        // is used as the title and the artist is unmodified.
        //
        // TODO(XXX): Disable splitting of artist/title in settings, i.e.
        // optionally don't split even if both title and artist are empty?
        // Some users might want to import the whole file name of untagged
        // files as the title without splitting the artist:
        //     https://www.mixxx.org/forums/viewtopic.php?f=3&t=12838
        // NOTE(uklotzde, 2019-09-26): Whoever needs this should simply set
        // splitArtistTitle = false here and compile their custom version!
        // It is not worth extending the settings and injecting them into
        // SoundSourceProxy for just a few people.
        const bool splitArtistTitle =
                trackMetadata.getTrackInfo().getArtist().trimmed().isEmpty();
        const auto fileInfo = m_pTrack->getFileInfo();
        kLogger.info()
                << "Parsing missing"
                << (splitArtistTitle ? "artist/title" : "title")
                << "from file name:"
                << fileInfo.location();
        if (trackMetadata.refTrackInfo().parseArtistTitleFromFileName(
                    fileInfo.fileName(), splitArtistTitle)) {
            // Pretend that metadata import succeeded
            metadataImportResult = mixxx::MetadataSource::ImportResult::Succeeded;
        }
    }

    // Do not continue with unknown and maybe invalid metadata!
    if (metadataImportResult != mixxx::MetadataSource::ImportResult::Succeeded) {
        return UpdateTrackFromSourceResult::MetadataImportFailed;
    }
    DEBUG_ASSERT(sourceSynchronizedAt.isValid());

    m_pTrack->replaceMetadataFromSource(
            std::move(trackMetadata),
            sourceSynchronizedAt);

    const bool pendingBeatsImport =
            m_pTrack->getBeatsImportStatus() == Track::ImportStatus::Pending;
    const bool pendingCueImport =
            m_pTrack->getCueImportStatus() == Track::ImportStatus::Pending;
    if (pendingBeatsImport || pendingCueImport) {
        // Try to open the audio source once to determine the actual
        // stream properties for finishing the pending import.
        kLogger.debug()
                << "Opening audio source to finish import of beats/cues";
        const auto pAudioSource = openAudioSource();
        DEBUG_ASSERT(!pAudioSource ||
                m_pTrack->getBeatsImportStatus() ==
                        Track::ImportStatus::Complete);
        DEBUG_ASSERT(!pAudioSource ||
                m_pTrack->getCueImportStatus() ==
                        Track::ImportStatus::Complete);
        if (pAudioSource) {
            // Close open file handles
            pAudioSource->close();
        }
    }

    if (pCoverImg) {
        // If the pointer is not null then the cover art should be guessed
        auto coverInfo =
                CoverInfoGuesser().guessCoverInfo(
                        m_pTrack->getFileInfo(),
                        m_pTrack->getAlbum(),
                        *pCoverImg);
        DEBUG_ASSERT(coverInfo.source == CoverInfo::GUESSED);
        m_pTrack->setCoverInfo(coverInfo);
    }

    return UpdateTrackFromSourceResult::MetadataImportedAndUpdated;
}

bool SoundSourceProxy::openSoundSource(
        const mixxx::AudioSource::OpenParams& params) {
    auto openMode = mixxx::SoundSource::OpenMode::Strict;
    int attemptCount = 0;
    while (m_pProvider && m_pSoundSource) {
        ++attemptCount;
        const mixxx::SoundSource::OpenResult openResult =
                m_pSoundSource->open(openMode, params);
        if (openResult == mixxx::SoundSource::OpenResult::Succeeded) {
            if (m_pSoundSource->verifyReadable()) {
                return true;
            }
            kLogger.warning()
                    << "Failed to read file"
                    << getUrl().toString(QUrl::PreferLocalFile)
                    << "with provider"
                    << m_pProvider->getDisplayName();
            m_pSoundSource->close(); // cleanup
        } else {
            kLogger.warning()
                    << "Failed to open file"
                    << getUrl().toString(QUrl::PreferLocalFile)
                    << "with provider"
                    << m_pProvider->getDisplayName()
                    << "using mode"
                    << openMode;
            if (openMode == mixxx::SoundSource::OpenMode::Permissive) {
                if (openResult == mixxx::SoundSource::OpenResult::Failed) {
                    // Do NOT retry with the next SoundSource provider if the file
                    // itself seems to be the cause when opening still fails during
                    // the 2nd (= permissive) round.
                    return false;
                } else {
                    // Continue and give other providers the chance to open the file
                    // in turn.
                    DEBUG_ASSERT(openResult == mixxx::SoundSource::OpenResult::Aborted);
                }
            } else {
                // Continue and give other providers the chance to open the file
                // in turn, independent of why opening the file failed.
                DEBUG_ASSERT(openMode == mixxx::SoundSource::OpenMode::Strict);
            }
        }
        // Continue with the next available SoundSource provider
        auto nextProviderWithOpenModePair = nextProviderWithOpenMode(openMode);
        auto pNextProvider = std::move(nextProviderWithOpenModePair.first);
        if (!pNextProvider) {
            break;
        }
        openMode = nextProviderWithOpenModePair.second;
        m_pProvider.reset();
        m_pSoundSource.reset();
        initSoundSourceWithProvider(std::move(pNextProvider));
        // try again
    }
    // All available providers have returned OpenResult::Aborted when
    // getting here. m_pSoundSource might already be invalid/null!
    kLogger.warning()
            << "Giving up to open file"
            << getUrl().toString(QUrl::PreferLocalFile)
            << "after"
            << attemptCount
            << "unsuccessful attempts";
    return false;
}

mixxx::AudioSourcePointer SoundSourceProxy::openAudioSource(
        const mixxx::AudioSource::OpenParams& params) {
    VERIFY_OR_DEBUG_ASSERT(m_pTrack) {
        return nullptr;
    }
    if (!openSoundSource(params)) {
        return nullptr;
    }
    // Overwrite metadata with actual audio properties
    m_pTrack->updateStreamInfoFromSource(
            m_pSoundSource->getStreamInfo());
    return mixxx::AudioSourceTrackProxy::create(m_pTrack, m_pSoundSource);
}
