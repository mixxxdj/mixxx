#include <QApplication>
#include <QStandardPaths>

#include "sources/soundsourceproxy.h"

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

#include "library/coverartcache.h"
#include "library/coverartutils.h"
#include "track/globaltrackcache.h"
#include "util/cmdlineargs.h"
#include "util/logger.h"
#include "util/regex.h"

//Static memory allocation
/*static*/ mixxx::SoundSourceProviderRegistry SoundSourceProxy::s_soundSourceProviders;
/*static*/ QStringList SoundSourceProxy::s_supportedFileNamePatterns;
/*static*/ QRegExp SoundSourceProxy::s_supportedFileNamesRegex;

namespace {

const mixxx::Logger kLogger("SoundSourceProxy");

} // anonymous namespace

// static
void SoundSourceProxy::registerSoundSourceProviders() {
    // Initialize built-in file types.
    // Fallback providers should be registered before specialized
    // providers to ensure that they are only after the specialized
    // provider failed to open a file. But the order of registration
    // only matters among providers with equal priority.
#ifdef __FFMPEG__
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
#ifdef __WV__
    s_soundSourceProviders.registerProvider(
            std::make_shared<mixxx::SoundSourceProviderWV>());
#endif
#ifdef __FAAD__
    s_soundSourceProviders.registerProvider(
            std::make_shared<mixxx::SoundSourceProviderM4A>());
#endif
#ifdef __COREAUDIO__
    s_soundSourceProviders.registerProvider(
            std::make_shared<mixxx::SoundSourceProviderCoreAudio>());
#endif
#ifdef __MEDIAFOUNDATION__
    s_soundSourceProviders.registerProvider(
            std::make_shared<mixxx::SoundSourceProviderMediaFoundation>());
#endif

    const QStringList supportedFileExtensions(
            s_soundSourceProviders.getRegisteredFileExtensions());
    if (kLogger.infoEnabled()) {
        for (const auto& supportedFileExtension : supportedFileExtensions) {
            kLogger.info() << "SoundSource providers for file extension" << supportedFileExtension;
            const QList<mixxx::SoundSourceProviderRegistration> registrationsForFileExtension(
                    s_soundSourceProviders.getRegistrationsForFileExtension(
                            supportedFileExtension));
            for (const auto& registration : registrationsForFileExtension) {
                kLogger.info() << " " << static_cast<int>(registration.getProviderPriority())
                               << ":" << registration.getProvider()->getName();
            }
        }
    }

    // Turn the file extension list into a [ "*.mp3", "*.wav", ... ] style string list
    s_supportedFileNamePatterns.clear();
    for (const auto& supportedFileExtension : supportedFileExtensions) {
        s_supportedFileNamePatterns += QString("*.%1").arg(supportedFileExtension);
    }

    // Build regular expression of supported file extensions
    QString supportedFileExtensionsRegex(
            RegexUtils::fileExtensionsRegex(supportedFileExtensions));
    s_supportedFileNamesRegex =
            QRegExp(supportedFileExtensionsRegex, Qt::CaseInsensitive);
}

// static
bool SoundSourceProxy::isUrlSupported(const QUrl& url) {
    return isFileSupported(TrackFile::fromUrl(url));
}

// static
bool SoundSourceProxy::isFileSupported(const QFileInfo& fileInfo) {
    return isFileNameSupported(fileInfo.fileName());
}

// static
bool SoundSourceProxy::isFileSupported(const TrackFile& trackFile) {
    return isFileNameSupported(trackFile.fileName());
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
        kLogger.warning() << "Unknown file type:" << url.toString();
        return QList<mixxx::SoundSourceProviderRegistration>();
    }

    QList<mixxx::SoundSourceProviderRegistration> registrationsForFileExtension(
            s_soundSourceProviders.getRegistrationsForFileExtension(
                    fileExtension));
    if (registrationsForFileExtension.isEmpty()) {
        kLogger.warning() << "Unsupported file type:" << url.toString();
    }

    return registrationsForFileExtension;
}

//static
TrackPointer SoundSourceProxy::importTemporaryTrack(
        TrackFile trackFile,
        SecurityTokenPointer pSecurityToken) {
    TrackPointer pTrack = Track::newTemporary(
            std::move(trackFile),
            std::move(pSecurityToken));
    // Lock the track cache while populating the temporary track
    // object to ensure that no metadata is exported into any file
    // while reading from this file. Since locking individual files
    // is not possible and the whole cache is locked.
    GlobalTrackCacheLocker locker;
    SoundSourceProxy(pTrack).updateTrackFromSource();
    return pTrack;
}

//static
QImage SoundSourceProxy::importTemporaryCoverImage(
        TrackFile trackFile,
        SecurityTokenPointer pSecurityToken) {
    TrackPointer pTrack = Track::newTemporary(
            std::move(trackFile),
            std::move(pSecurityToken));
    // Lock the track cache while populating the temporary track
    // object to ensure that no metadata is exported into any file
    // while reading from this file. Since locking individual files
    // is not possible and the whole cache is locked.
    GlobalTrackCacheLocker locker;
    return SoundSourceProxy(pTrack).importCoverImage();
}

//static
Track::ExportMetadataResult
SoundSourceProxy::exportTrackMetadataBeforeSaving(Track* pTrack) {
    DEBUG_ASSERT(pTrack);
    const auto trackFile = pTrack->getFileInfo();
    mixxx::MetadataSourcePointer pMetadataSource =
            SoundSourceProxy(trackFile.toUrl()).m_pSoundSource;
    if (pMetadataSource) {
        return pTrack->exportMetadata(pMetadataSource);
    } else {
        kLogger.warning()
                << "Unable to export track metadata into file"
                << trackFile.location();
        return Track::ExportMetadataResult::Skipped;
    }
}

SoundSourceProxy::SoundSourceProxy(
        TrackPointer pTrack)
        : m_pTrack(std::move(pTrack)),
          m_url(m_pTrack ? m_pTrack->getFileInfo().toUrl() : QUrl()),
          m_soundSourceProviderRegistrations(findSoundSourceProviderRegistrations(m_url)),
          m_soundSourceProviderRegistrationIndex(0) {
    initSoundSource();
}

SoundSourceProxy::SoundSourceProxy(
        const QUrl& url)
        : m_url(url),
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
                kLogger.warning() << "No SoundSourceProvider for file"
                                  << getUrl().toString();
            }
            // Failure
            return;
        }
        m_pSoundSource = pProvider->newSoundSource(m_url);
        if (!m_pSoundSource) {
            kLogger.warning() << "SoundSourceProvider"
                              << pProvider->getName()
                              << "failed to create a SoundSource for file"
                              << getUrl().toString();
            // Switch to next provider...
            nextSoundSourceProvider();
            // ...and continue loop
            DEBUG_ASSERT(!m_pSoundSource);
        } else {
            if (kLogger.debugEnabled()) {
                kLogger.debug() << "SoundSourceProvider"
                                << pProvider->getName()
                                << "created a SoundSource for file"
                                << getUrl().toString()
                                << "of type"
                                << m_pSoundSource->getType();
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
bool parseMetadataFromFileName(mixxx::TrackMetadata* pTrackMetadata, QString fileName) {
    fileName.replace("_", " ");
    QString titleWithFileType;
    bool parsed = false;
    if (fileName.count('-') == 1) {
        if (pTrackMetadata->getTrackInfo().getArtist().isEmpty()) {
            const QString artist(fileName.section('-', 0, 0).trimmed());
            if (!artist.isEmpty()) {
                pTrackMetadata->refTrackInfo().setArtist(artist);
                parsed = true;
            }
        }
        titleWithFileType = fileName.section('-', 1, 1).trimmed();
    } else {
        titleWithFileType = fileName.trimmed();
    }
    if (pTrackMetadata->getTrackInfo().getTitle().isEmpty()) {
        const QString title(titleWithFileType.section('.', 0, -2).trimmed());
        if (!title.isEmpty()) {
            pTrackMetadata->refTrackInfo().setTitle(title);
            parsed = true;
        }
    }
    return parsed;
}
} // anonymous namespace

void SoundSourceProxy::updateTrackFromSource(
        ImportTrackMetadataMode importTrackMetadataMode) const {
    DEBUG_ASSERT(m_pTrack);

    if (getUrl().isEmpty()) {
        // Silently skip tracks without a corresponding file
        return; // abort
    }
    if (!m_pSoundSource) {
        kLogger.warning()
                << "Unable to update track from unsupported file type"
                << getUrl().toString();
        return; // abort
    }

    // The SoundSource provides the actual type of the corresponding file
    m_pTrack->setType(m_pSoundSource->getType());

    // Use the existing track metadata as default values. Otherwise
    // existing values in the library would be overwritten with empty
    // values if the corresponding file tags are missing. Depending
    // on the file type some kind of tags might even not be supported
    // at all and this information would get lost entirely otherwise!
    mixxx::TrackMetadata trackMetadata;
    bool metadataSynchronized = false;
    m_pTrack->getTrackMetadata(&trackMetadata, &metadataSynchronized);
    // If the file tags have already been parsed at least once, the
    // existing track metadata should not be updated implicitly, i.e.
    // if the user did not explicitly choose to (re-)import metadata
    // explicitly from this file.
    if (metadataSynchronized &&
            (importTrackMetadataMode == ImportTrackMetadataMode::Once)) {
        if (kLogger.debugEnabled()) {
            kLogger.debug()
                    << "Skip importing of track metadata and embedded cover art from file"
                    << getUrl().toString();
        }
        return; // abort
    }

    // Embedded cover art is imported together with the track's metadata.
    // But only if the user has not selected external cover art for this
    // track!
    QImage coverImg;
    DEBUG_ASSERT(coverImg.isNull());
    QImage* pCoverImg; // pointer is also used as a flag
    const CoverInfoRelative coverInfoOld = m_pTrack->getCoverInfo();
    // Only re-import cover art if it is save to update, e.g. never
    // discard a users's custom choice! We are using a whitelisting
    // filter here that explicitly checks all valid preconditions
    // when it is permissible to update/replace existing cover art.
    if (((coverInfoOld.source == CoverInfo::UNKNOWN) || (coverInfoOld.source == CoverInfo::GUESSED)) &&
            ((coverInfoOld.type == CoverInfo::NONE) || (coverInfoOld.type == CoverInfo::METADATA))) {
        // Should import and update embedded cover art
        pCoverImg = &coverImg;
    } else {
        if (kLogger.debugEnabled()) {
            kLogger.debug()
                    << "Skip importing of embedded cover art from file"
                    << getUrl().toString();
        }
        // Skip import of embedded cover art
        pCoverImg = nullptr;
    }

    // Parse the tags stored in the audio file
    auto metadataImported =
            m_pSoundSource->importTrackMetadataAndCoverImage(
                    &trackMetadata, pCoverImg);
    if (metadataImported.first == mixxx::MetadataSource::ImportResult::Failed) {
        kLogger.warning()
                << "Failed to import track metadata"
                << (pCoverImg ? "and embedded cover art" : "")
                << "from file"
                << getUrl().toString();
        // Continue for now, but the abort may follow shortly if the
        // track already has metadata (see below)
    }
    if (metadataSynchronized) {
        // Metadata has been synchronized successfully at least
        // once in the past. Only overwrite this information if
        // new data has actually been imported, otherwise abort
        // and preserve the existing data!
        if (metadataImported.first != mixxx::MetadataSource::ImportResult::Succeeded) {
            return; // abort
        }
        if (kLogger.debugEnabled()) {
            kLogger.debug()
                    << "Updating track metadata"
                    << (pCoverImg ? "and embedded cover art" : "")
                    << "from file"
                    << getUrl().toString();
        }
    } else {
        DEBUG_ASSERT(pCoverImg);
        if (kLogger.debugEnabled()) {
            kLogger.debug()
                    << "Initializing track metadata and embedded cover art from file"
                    << getUrl().toString();
        }
    }

    // Fallback: If artist or title fields are blank then try to populate
    // them from the file name. This might happen if tags are unavailable,
    // unreadable, or partially/completely missing.
    if (trackMetadata.getTrackInfo().getArtist().isEmpty() ||
            trackMetadata.getTrackInfo().getTitle().isEmpty()) {
        kLogger.info()
                << "Adding missing artist/title from file name"
                << getUrl().toString();
        const auto trackFile = m_pTrack->getFileInfo();
        if (parseMetadataFromFileName(&trackMetadata, trackFile.fileName()) &&
                metadataImported.second.isNull()) {
            // Since this is also some kind of metadata import, we mark the
            // track's metadata as synchronized with the time stamp of the file.
            metadataImported.second = trackFile.fileLastModified();
        }
    }

    m_pTrack->setTrackMetadata(trackMetadata, metadataImported.second);

    if (pCoverImg) {
        // If the pointer is not null then the cover art should be guessed
        // from the embedded metadata
        CoverInfoRelative coverInfoNew;
        DEBUG_ASSERT(coverInfoNew.coverLocation.isNull());
        if (pCoverImg->isNull()) {
            // Cover art will be cleared
            DEBUG_ASSERT(coverInfoNew.source == CoverInfo::UNKNOWN);
            DEBUG_ASSERT(coverInfoNew.type == CoverInfo::NONE);
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "No embedded cover art found in file"
                        << getUrl().toString();
            }
        } else {
            coverInfoNew.source = CoverInfo::GUESSED;
            coverInfoNew.type = CoverInfo::METADATA;
            // TODO(XXX) here we may introduce a duplicate hash code
            coverInfoNew.hash = CoverArtUtils::calculateHash(coverImg);
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Embedded cover art found in file"
                        << getUrl().toString();
            }
        }
        m_pTrack->setCoverInfo(coverInfoNew);
    }
}

mixxx::MetadataSource::ImportResult SoundSourceProxy::importTrackMetadata(mixxx::TrackMetadata* pTrackMetadata) const {
    if (m_pSoundSource) {
        return m_pSoundSource->importTrackMetadataAndCoverImage(pTrackMetadata, nullptr).first;
    } else {
        return mixxx::MetadataSource::ImportResult::Unavailable;
    }
}

QImage SoundSourceProxy::importCoverImage() const {
    if (m_pSoundSource) {
        QImage coverImg;
        if (m_pSoundSource->importTrackMetadataAndCoverImage(nullptr, &coverImg).first ==
                mixxx::MetadataSource::ImportResult::Succeeded) {
            return coverImg;
        }
    }
    // Failed ore unavailable
    return QImage();
}

mixxx::AudioSourcePointer SoundSourceProxy::openAudioSource(const mixxx::AudioSource::OpenParams& params) {
    DEBUG_ASSERT(m_pTrack);
    auto openMode = mixxx::SoundSource::OpenMode::Strict;
    while (m_pSoundSource && !m_pAudioSource) {
        // NOTE(uklotzde): Log unconditionally (with debug level) to
        // identify files in the log file that might have caused a
        // crash while importing metadata or decoding audio subsequently.
        kLogger.debug() << "Opening file"
                        << getUrl().toString()
                        << "with provider"
                        << getSoundSourceProvider()->getName()
                        << "using mode"
                        << openMode;
        const mixxx::SoundSource::OpenResult openResult =
                m_pSoundSource->open(openMode, params);
        if ((openResult == mixxx::SoundSource::OpenResult::Aborted) ||
                ((openMode == mixxx::SoundSource::OpenMode::Strict) && (openResult == mixxx::SoundSource::OpenResult::Failed))) {
            kLogger.warning() << "Unable to open file"
                              << getUrl().toString()
                              << "with provider"
                              << getSoundSourceProvider()->getName()
                              << "using mode"
                              << openMode;
            // Continue with the next SoundSource provider
            nextSoundSourceProvider();
            if (!getSoundSourceProvider() && (openMode == mixxx::SoundSource::OpenMode::Strict)) {
                // No provider was able to open the source in Strict mode.
                // Retry to open the file in Permissive mode starting with
                // the first provider...
                m_soundSourceProviderRegistrationIndex = 0;
                openMode = mixxx::SoundSource::OpenMode::Permissive;
            }
            initSoundSource();
            continue; // try again
        }
        if ((openResult == mixxx::SoundSource::OpenResult::Succeeded) && m_pSoundSource->verifyReadable()) {
            m_pAudioSource = mixxx::AudioSourceTrackProxy::create(m_pTrack, m_pSoundSource);
            DEBUG_ASSERT(m_pAudioSource);
            if (m_pAudioSource->frameIndexRange().empty()) {
                kLogger.warning() << "File is empty"
                                  << getUrl().toString();
            }
            // Overwrite metadata with actual audio properties
            if (m_pTrack) {
                DEBUG_ASSERT(m_pAudioSource->channelCount().valid());
                m_pTrack->setChannels(m_pAudioSource->channelCount());
                DEBUG_ASSERT(m_pAudioSource->sampleRate().valid());
                m_pTrack->setSampleRate(m_pAudioSource->sampleRate());
                if (m_pAudioSource->hasDuration()) {
                    // optional property
                    m_pTrack->setDuration(m_pAudioSource->getDuration());
                }
                if (m_pAudioSource->bitrate() != mixxx::AudioSource::Bitrate()) {
                    // optional property
                    m_pTrack->setBitrate(m_pAudioSource->bitrate());
                }
            }
        } else {
            kLogger.warning() << "Failed to open file"
                              << getUrl().toString()
                              << "with provider"
                              << getSoundSourceProvider()->getName();
            if (openResult == mixxx::SoundSource::OpenResult::Succeeded) {
                m_pSoundSource->close(); // cleanup
            }
            // Do NOT retry with the next SoundSource provider if the file
            // itself is the cause!
            DEBUG_ASSERT(!m_pAudioSource);
        }
        return m_pAudioSource; // either success or failure
    }
    // All available providers have returned OpenResult::Aborted when
    // getting here. m_pSoundSource might already be invalid/null!
    kLogger.warning() << "Unable to decode file"
                      << getUrl().toString();
    DEBUG_ASSERT(!m_pAudioSource);
    return m_pAudioSource;
}

void SoundSourceProxy::closeAudioSource() {
    if (m_pAudioSource) {
        DEBUG_ASSERT(m_pSoundSource);
        m_pSoundSource->close();
        m_pAudioSource = mixxx::AudioSourcePointer();
        if (kLogger.debugEnabled()) {
            kLogger.debug() << "Closed AudioSource for file"
                            << getUrl().toString();
        }
    }
}
