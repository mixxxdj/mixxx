#include "engine/cachingreader/cachingreaderworker.h"

#include <QAtomicInt>
#include <QBuffer>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QRegularExpression>
#include <QStorageInfo>
#include <QTemporaryFile>
#include <QtDebug>

#include "analyzer/analyzersilence.h"
#include "moc_cachingreaderworker.cpp"
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "util/compatibility/qmutex.h"
#include "util/event.h"
#include "util/fifo.h"
#include "util/logger.h"
#include "util/span.h"

namespace {
const bool sDebugCachingReaderWorker = false;
mixxx::Logger kLogger("CachingReaderWorker");

// we need the last silence frame and the first sound frame
constexpr SINT kNumSoundFrameToVerify = 2;
static QRegularExpression s_filenameSanitizeRegex(R"([\/\\\:\*\?\"\<\>\|])");
} // anonymous namespace

CachingReaderWorker::CachingReaderWorker(
        const QString& group,
        FIFO<CachingReaderChunkReadRequest>* pChunkReadRequestFIFO,
        FIFO<ReaderStatusUpdate>* pReaderStatusFIFO,
        mixxx::audio::ChannelCount maxSupportedChannel)
        : m_group(group),
          m_tag(QString("CachingReaderWorker %1").arg(m_group)),
          m_pChunkReadRequestFIFO(pChunkReadRequestFIFO),
          m_pReaderStatusFIFO(pReaderStatusFIFO),
          m_maxSupportedChannel(maxSupportedChannel) {
}

QHash<QString, CachingReaderWorker::TrackFileCacheTrackEntry>
        CachingReaderWorker::s_trackFileCacheTracks;
QMutex CachingReaderWorker::s_trackFileCacheTracksMutex;
QString CachingReaderWorker::gSessionPrefix = QString("MixxxTemp_%1_")
                                                      .arg(QDateTime::currentMSecsSinceEpoch());

ReaderStatusUpdate CachingReaderWorker::processReadRequest(
        const CachingReaderChunkReadRequest& request) {
    CachingReaderChunk* pChunk = request.chunk;
    DEBUG_ASSERT(pChunk);

    // EVE
    // kLogger.debug() << m_group << "Processing read request for chunk"
    //                << request.chunk->getIndex()
    //                << "frames:" << request.chunk->frameIndexRange(m_pAudioSource);

    // auto startTime = QDateTime::currentDateTime();
    //  EVE

    // Before trying to read any data we need to check if the audio source
    // is available and if any audio data that is needed by the chunk is
    // actually available.
    auto chunkFrameIndexRange = pChunk->frameIndexRange(m_pAudioSource);
    DEBUG_ASSERT(!m_pAudioSource ||
            chunkFrameIndexRange.isSubrangeOf(m_pAudioSource->frameIndexRange()));
    if (chunkFrameIndexRange.empty()) {
        ReaderStatusUpdate result;
        result.init(CHUNK_READ_INVALID, pChunk, m_pAudioSource ? m_pAudioSource->frameIndexRange() : mixxx::IndexRange());
        return result;
    }

    // Try to read the data required for the chunk from the audio source
    const mixxx::IndexRange bufferedFrameIndexRange = pChunk->bufferSampleFrames(
            m_pAudioSource,
            mixxx::SampleBuffer::WritableSlice(m_tempReadBuffer));

    // EVE
    // auto duration = startTime.msecsTo(QDateTime::currentDateTime());
    // kLogger.debug() << m_group << "Chunk read completed in" << duration << "ms"
    //                << "buffered frames:" << bufferedFrameIndexRange;
    // EVE

    DEBUG_ASSERT(!m_pAudioSource ||
            bufferedFrameIndexRange.isSubrangeOf(m_pAudioSource->frameIndexRange()));
    // The readable frame range might have changed
    chunkFrameIndexRange = intersect(chunkFrameIndexRange, m_pAudioSource->frameIndexRange());
    DEBUG_ASSERT(bufferedFrameIndexRange.empty() ||
            bufferedFrameIndexRange.isSubrangeOf(chunkFrameIndexRange));

    ReaderStatus status = bufferedFrameIndexRange.empty() ? CHUNK_READ_EOF : CHUNK_READ_SUCCESS;
    if (bufferedFrameIndexRange != chunkFrameIndexRange) {
        kLogger.warning()
                << m_group
                << "Failed to read chunk samples for frame index range:"
                << "expected =" << chunkFrameIndexRange
                << ", actual =" << bufferedFrameIndexRange;
        if (bufferedFrameIndexRange.empty()) {
            status = CHUNK_READ_INVALID; // overwrite EOF (see above)
        }
    }

    // This call here assumes that the caching reader will read the first sound cue at
    // one of the first chunks. The check serves as a sanity check to ensure that the
    // sample data has not changed since it has ben analyzed. This could happen because
    // of a change in actual audio data or because the file was decoded using a different
    // decoder
    // This is part of a first prove of concept and needs to be replaces with a different
    // solution which is still under discussion. This might be also extended
    // to further checks whether a automatic offset adjustment is possible or a the
    // sample position metadata shall be treated as outdated.
    // Failures of the sanity check only result in an entry into the log at the moment.
    verifyFirstSound(pChunk, m_pAudioSource->getSignalInfo().getChannelCount());

    ReaderStatusUpdate result;
    result.init(status, pChunk, m_pAudioSource ? m_pAudioSource->frameIndexRange() : mixxx::IndexRange());
    return result;
}

// WARNING: Always called from a different thread (GUI)
#ifdef __STEM__
void CachingReaderWorker::newTrack(TrackPointer pTrack, mixxx::StemChannelSelection stemMask) {
#else
void CachingReaderWorker::newTrack(TrackPointer pTrack) {
#endif
    {
        const auto locker = lockMutex(&m_newTrackMutex);
#ifdef __STEM__
        m_pNewTrack = NewTrackRequest{
                pTrack,
                stemMask};
#else
        m_pNewTrack = pTrack;
#endif
        m_newTrackAvailable.storeRelease(1);
    }
    workReady();
}

void CachingReaderWorker::run() {
    // EVE
    // kLogger.debug() << m_group << "Worker thread started";
    // EVE

    // the id of this thread, for debugging purposes
    static auto lastId = QAtomicInt(0);
    const auto id = lastId.fetchAndAddRelaxed(1) + 1;
    QThread::currentThread()->setObjectName(
            QStringLiteral("CachingReaderWorker ") + QString::number(id));

    Event::start(m_tag);
    while (!m_stop.loadAcquire()) {
        // Request is initialized by reading from FIFO
        CachingReaderChunkReadRequest request;
        if (m_newTrackAvailable.loadAcquire()) {
            // EVE
            // kLogger.debug() << m_group << "New track available";
            // EVE

#ifdef __STEM__
            NewTrackRequest pLoadTrack;
#else
            TrackPointer pLoadTrack;
#endif
            { // locking scope
                const auto locker = lockMutex(&m_newTrackMutex);
                pLoadTrack = m_pNewTrack;
                m_newTrackAvailable.storeRelease(0);
            } // implicitly unlocks the mutex
#ifdef __STEM__
            if (pLoadTrack.track) {
                // in this case the engine is still running with the old track
                loadTrack(pLoadTrack.track, pLoadTrack.stemMask);
#else
            if (pLoadTrack) {
                // in this case the engine is still running with the old track
                loadTrack(pLoadTrack);
#endif
            } else {
                // here, the engine is already stopped
                unloadTrack();
            }
        } else if (m_pChunkReadRequestFIFO->read(&request, 1) == 1) {
            // EVE
            // kLogger.trace() << m_group << "Read request from FIFO for chunk"
            //                << request.chunk->getIndex();
            // EVE
            // Read the requested chunk and send the result
            const ReaderStatusUpdate update = processReadRequest(request);
            m_pReaderStatusFIFO->writeBlocking(&update, 1);
        } else {
            Event::end(m_tag);
            m_semaRun.acquire();
            Event::start(m_tag);
        }
    }
    kLogger.debug() << m_group << "Worker thread stopping";
}

void CachingReaderWorker::discardAllPendingRequests() {
    CachingReaderChunkReadRequest request;
    while (m_pChunkReadRequestFIFO->read(&request, 1) == 1) {
        const auto update = ReaderStatusUpdate::readDiscarded(request.chunk);
        m_pReaderStatusFIFO->writeBlocking(&update, 1);
    }
}

void CachingReaderWorker::closeAudioSource() {
    discardAllPendingRequests();

    if (m_pAudioSource) {
        // Closes open file handles of the old track.
        m_pAudioSource->close();
        m_pAudioSource.reset();
    }

    // Clean up temporary TrackFileCache file, if used
    if (m_tmpTrackFileCacheFile) {
        m_tmpTrackFileCacheFile->close();
        delete m_tmpTrackFileCacheFile;
        m_tmpTrackFileCacheFile = nullptr;
    }

    // This function has to be called with the engine stopped only
    // to avoid collecting new requests for the old track
    DEBUG_ASSERT(!m_pChunkReadRequestFIFO->readAvailable());
}

// TrackFileCache
void CachingReaderWorker::setTrackFileCacheConfig(bool enabled,
        const QString& trackFileCacheDiskPath,
        int maxTrackFileCacheSizeMB,
        bool decksEnabled,
        bool samplersEnabled,
        bool previewEnabled) {
    m_trackFileCacheEnabled = enabled;
    m_trackFileCacheDiskPath = trackFileCacheDiskPath;
    m_maxTrackFileCacheSizeMB = maxTrackFileCacheSizeMB;
    m_trackFileCacheDecks = decksEnabled;
    m_trackFileCacheSamplers = samplersEnabled;
    m_trackFileCachePreview = previewEnabled;
    if (sDebugCachingReaderWorker) {
        kLogger.debug() << m_group << "[TrackFileCache] Config - Enabled:"
                        << m_trackFileCacheEnabled
                        << "Path:" << m_trackFileCacheDiskPath
                        << "MaxSize:" << m_maxTrackFileCacheSizeMB << "MB"
                        << "TrackFileCache for Decks: " << m_trackFileCacheDecks
                        << "TrackFileCache for Samplers: "
                        << m_trackFileCacheSamplers
                        << "TrackFileCache for PreviexDeck: "
                        << m_trackFileCachePreview;
    }
}

struct TrackFileCacheTrackEntry {
    QString group;
    QString filePath;
};

static QHash<QString, TrackFileCacheTrackEntry> s_trackFileCacheTracks;
static QHash<QString, QSet<QString>> s_fileToGroupsMap;
static QMutex s_trackFileCacheTracksMutex;

static QString sanitizeFileNamePart(const QString& str) {
    QString s = str;
    // Replace any character that is invalid in file names with underscore
    s.replace(s_filenameSanitizeRegex, "_");
    return s;
}

// Generate session prefix once per Mixxx-session
static QString gSessionPrefix = QString("MixxxTemp_%1_")
                                        .arg(QDateTime::currentMSecsSinceEpoch());

// Helper function to check if a RAM file is in use in another group
static bool isTrackFileCacheFileUsedByOtherGroups(
        const QString& filePath, const QString& currentGroup) {
    QMutexLocker locker(&s_trackFileCacheTracksMutex);
    return s_fileToGroupsMap.contains(filePath) &&
            (s_fileToGroupsMap[filePath].size() > 1 ||
                    !s_fileToGroupsMap[filePath].contains(currentGroup));
}

// Helper function to remove TrackFileCache file if not in use in another group
static void cleanupTrackFileCacheFileIfUnused(const QString& filePath) {
    QMutexLocker locker(&s_trackFileCacheTracksMutex);

    // Check if file is in use
    bool isUsed = false;
    for (const auto& entry : std::as_const(s_trackFileCacheTracks)) {
        if (entry.filePath == filePath) {
            isUsed = true;
            break;
        }
    }

    // Remove file if not in use
    if (!isUsed) {
        QFile file(filePath);
        if (file.exists()) {
            if (file.remove()) {
                kLogger.debug() << "[TrackFileCache] -> Removed unused "
                                   "TrackFileCache file:"
                                << filePath;
            } else {
                kLogger.warning() << "[TrackFileCache] -> Failed to remove "
                                     "TrackFileCache file:"
                                  << filePath;
            }
        }
    }
}

// Update TrackFileCache track for group
static void updateTrackFileCacheTrackUsage(const QString& group, const QString& filePath) {
    QString fileToRemove;
    {
        QMutexLocker locker(&s_trackFileCacheTracksMutex);
        if (s_trackFileCacheTracks.contains(group)) {
            QString oldFilePath = s_trackFileCacheTracks[group].filePath;
            s_trackFileCacheTracks.remove(group);

            // Check if file should be removed (if not used by others)
            bool shouldRemove = !oldFilePath.isEmpty() &&
                    !isTrackFileCacheFileUsedByOtherGroups(oldFilePath, group);
            if (shouldRemove) {
                fileToRemove = oldFilePath;
            }
        }

        // Add new entry
        if (!filePath.isEmpty()) {
            s_trackFileCacheTracks.insert(group, {group, filePath});
        }
    }

    // Remove file outside critical section
    if (!fileToRemove.isEmpty()) {
        QFile::remove(fileToRemove);
    }
}

// Remove group from TrackFileCache track usage (when deck/sampler is unloaded)
static void removeTrackFileCacheTrackUsage(const QString& group) {
    QMutexLocker locker(&s_trackFileCacheTracksMutex);

    if (s_trackFileCacheTracks.contains(group)) {
        const QString filePath = s_trackFileCacheTracks[group].filePath;
        s_trackFileCacheTracks.remove(group);

        // Clean up file if it's not used by other groups
        locker.unlock();
        cleanupTrackFileCacheFileIfUnused(filePath);
    }
}

#ifdef __STEM__
void CachingReaderWorker::loadTrack(
        const TrackPointer& pTrack, mixxx::StemChannelSelection stemMask) {
#else
void CachingReaderWorker::loadTrack(const TrackPointer& pTrack) {
#endif
    // Notify UI / engine that track loading started
    emit trackLoading();

    // Close any previous audio source and clean up previous RAM usage
    closeAudioSource();

    // Remove previous RAM track usage for this group
    removeTrackFileCacheTrackUsage(m_group);

    if (!pTrack->getFileInfo().checkFileExists()) {
        kLogger.warning() << m_group << "File not found" << pTrack->getFileInfo();
        const auto update = ReaderStatusUpdate::trackUnloaded();
        m_pReaderStatusFIFO->writeBlocking(&update, 1);
        emit trackLoadFailed(pTrack,
                tr("The file '%1' could not be found.")
                        .arg(QDir::toNativeSeparators(pTrack->getLocation())));
        return;
    }

    // Check if TrackFileCache play is enabled globally
    if (!m_trackFileCacheEnabled) {
        if (sDebugCachingReaderWorker) {
            kLogger.debug() << "[TrackFileCache] -> Player: " << m_group
                            << " TrackFileCache disabled in preferences, playing "
                               "file from original location";
        }
        // Skip TrackFileCache and load from original location
        TrackPointer trackToOpen = pTrack;
        trackToOpen->setURL(pTrack->getURL());
        updateTrackFileCacheTrackUsage(m_group, "");
        openAudioSource(trackToOpen, stemMask);
        return;
    }

    // Check Deck/Sampler/PreviewDeck preferences
    bool componentEnabled = true;
    QString componentType;

    if (m_group.contains("Channel", Qt::CaseInsensitive)) {
        componentEnabled = m_trackFileCacheDecks;
        componentType = "Deck";
    } else if (m_group.contains("Sampler", Qt::CaseInsensitive)) {
        componentEnabled = m_trackFileCacheSamplers;
        componentType = "Sampler";
    } else if (m_group.contains("Preview", Qt::CaseInsensitive)) {
        componentEnabled = m_trackFileCachePreview;
        componentType = "PreviewDeck";
    }

    if (!componentEnabled) {
        if (sDebugCachingReaderWorker) {
            kLogger.debug() << "[TrackFileCache] -> Player: " << m_group
                            << " TrackFileCache disabled for " << componentType
                            << ", playing file from original location";
        }
        // Skip TrackFileCache and load from original location
        TrackPointer trackToOpen = pTrack;
        trackToOpen->setURL(pTrack->getURL());
        updateTrackFileCacheTrackUsage(m_group, "");
        openAudioSource(trackToOpen, stemMask);
        return;
    }

    // -> 1: Setup TrackFileCache storage ///
    QString tmpPath = m_trackFileCacheDiskPath;
    QDir tmpDir(tmpPath);

    // Create directory if it doesn't exist
    if (!tmpDir.exists()) {
        if (!tmpDir.mkpath(tmpPath)) {
            kLogger.warning() << "[TrackFileCache] -> Failed to create directory:" << tmpPath;
            // Fall back to original file
            TrackPointer trackToOpen = pTrack;
            trackToOpen->setURL(pTrack->getURL());
            updateTrackFileCacheTrackUsage(m_group, "");
            openAudioSource(trackToOpen, stemMask);
            return;
        }
        if (sDebugCachingReaderWorker) {
            kLogger.debug() << "[TrackFileCache] -> Created directory:" << tmpPath;
        }
    }

    QStringList oldFiles = tmpDir.entryList(QStringList() << "MixxxTemp_*", QDir::Files);
    // keep current session files, delete files from other (earlier) session
    for (const QString& f : std::as_const(oldFiles)) {
        if (!f.startsWith(gSessionPrefix)) {
            QFile::remove(tmpDir.filePath(f));
        }
    }

    QString artistSafe = sanitizeFileNamePart(pTrack->getArtist());
    QString titleSafe = sanitizeFileNamePart(pTrack->getTitle());

    QString combined = artistSafe + "-" + titleSafe;
    if (combined.length() > 50) {
        combined = combined.left(50);
    }

    QString trackIdSafe = sanitizeFileNamePart(pTrack->getId().toString());
    QString trackFileCacheFileName =
            tmpPath + gSessionPrefix + trackIdSafe + "_" + combined + ".tmp";
    bool useTrackFileCacheCopy = false;

    QFile trackFileCacheFile(trackFileCacheFileName);
    if (trackFileCacheFile.exists()) {
        if (sDebugCachingReaderWorker) {
            kLogger.debug()
                    << "[TrackFileCache] -> Player: " << m_group
                    << " Reusing existing TrackFileCache file for track:"
                    << trackFileCacheFileName;
        }
        useTrackFileCacheCopy = true;
    } else {
        qint64 trackSize = QFileInfo(pTrack->getFileInfo().location()).size();
        qint64 maxTrackFileCacheBytes =
                static_cast<qint64>(m_maxTrackFileCacheSizeMB) * 1024 * 1024;

        // Calculate the size of actual Mixxx session files in TrackFileCache directory
        qint64 mixxxSessionSize = 0;
        QDir tmpDir(tmpPath);
        if (tmpDir.exists()) {
            QStringList sessionFiles = tmpDir.entryList(
                    QStringList() << gSessionPrefix + "*", QDir::Files);
            for (const QString& filename : std::as_const(sessionFiles)) {
                QFileInfo fileInfo(tmpDir.filePath(filename));
                mixxxSessionSize += fileInfo.size();
            }
        }
        if (sDebugCachingReaderWorker) {
            kLogger.debug() << "[TrackFileCache] -> Mixxx session size:"
                            << mixxxSessionSize / (1024 * 1024)
                            << "MB, Track:" << trackSize / (1024 * 1024)
                            << "MB, Max allowed:" << m_maxTrackFileCacheSizeMB << "MB";
        }
        // Check if adding this track would exceed the Mixxx-specific limit
        if ((mixxxSessionSize + trackSize) > maxTrackFileCacheBytes) {
            kLogger.warning() << "[TrackFileCache] -> Player: " << m_group
                              << " TrackFileCache copy would exceed Mixxx size limit. Track:"
                              << trackSize / (1024 * 1024) << "MB, Mixxx usage:"
                              << mixxxSessionSize / (1024 * 1024) << "MB, Max:"
                              << m_maxTrackFileCacheSizeMB << "MB";
            useTrackFileCacheCopy = false;
        } else {
            useTrackFileCacheCopy = true;
            if (!trackFileCacheFile.open(QIODevice::WriteOnly)) {
                kLogger.warning()
                        << "[TrackFileCache] -> Player: " << m_group
                        << " Cannot open TrackFileCache file for writing:"
                        << trackFileCacheFileName;
                useTrackFileCacheCopy = false;
            } else {
                QFile originalFile(pTrack->getFileInfo().location());
                if (!originalFile.open(QIODevice::ReadOnly)) {
                    kLogger.warning()
                            << "[TrackFileCache] -> Player: " << m_group
                            << " Cannot read original track:" << pTrack->getLocation();
                    useTrackFileCacheCopy = false;
                } else {
                    trackFileCacheFile.write(originalFile.readAll());
                    trackFileCacheFile.flush();
                    trackFileCacheFile.close();
                    originalFile.close();
                    if (sDebugCachingReaderWorker) {
                        kLogger.debug()
                                << "[TrackFileCache] -> Player: " << m_group
                                << " Track copied to TrackFileCache directory:"
                                << trackFileCacheFileName;
                    }
                }
            }
        }
    }

    TrackPointer trackToOpen = pTrack;
    if (useTrackFileCacheCopy) {
        trackToOpen->setURL(QUrl::fromLocalFile(trackFileCacheFileName).toString());
        m_trackFileCacheFilesInUse.insert(trackFileCacheFileName);
        updateTrackFileCacheTrackUsage(m_group, trackFileCacheFileName);
    } else {
        trackToOpen->setURL(pTrack->getURL());
        updateTrackFileCacheTrackUsage(m_group, "");
    }

    // -> 2: Open AudioSource ///
    openAudioSource(trackToOpen, stemMask);
}

// Helper function to open audio source (extracted for clarity)
void CachingReaderWorker::openAudioSource(const TrackPointer& trackToOpen,
#ifdef __STEM__
        mixxx::StemChannelSelection stemMask) {
#else
) {
#endif
    mixxx::AudioSource::OpenParams config;
    config.setChannelCount(m_maxSupportedChannel);

#ifdef __STEM__
    config.setStemMask(stemMask);
#endif

    m_pAudioSource = SoundSourceProxy(trackToOpen).openAudioSource(config);
    if (!m_pAudioSource) {
        kLogger.warning() << m_group << "Failed to open track:" << trackToOpen->getFileInfo();
        emit trackLoadFailed(trackToOpen, tr("Failed to open track."));
        return;
    }

    // -> Step 3: Validate audio source ///
    if (m_pAudioSource->getSignalInfo().getChannelCount() < mixxx::audio::ChannelCount::mono() ||
            m_pAudioSource->getSignalInfo().getChannelCount() > m_maxSupportedChannel) {
        kLogger.warning() << m_group
                          << "Track contains unsupported number of channels:"
                          << trackToOpen->getFileInfo();
        m_pAudioSource.reset();
        emit trackLoadFailed(trackToOpen, tr("Unsupported number of channels."));
        return;
    }

    if (m_pAudioSource->frameIndexRange().empty()) {
        kLogger.warning() << m_group << "Empty track:" << trackToOpen->getFileInfo();
        m_pAudioSource.reset();
        emit trackLoadFailed(trackToOpen, tr("Track is empty."));
        return;
    }

    // -> 4: Prepare temp buffer for chunking ///
    const SINT tempReadBufferSize =
            m_pAudioSource->getSignalInfo().frames2samples(CachingReaderChunk::kFrames);
    if (m_tempReadBuffer.size() != tempReadBufferSize) {
        mixxx::SampleBuffer(tempReadBufferSize).swap(m_tempReadBuffer);
    }

    // -> 5: Notify engine/UI ///
    const auto update = ReaderStatusUpdate::trackLoaded(m_pAudioSource->frameIndexRange());
    m_pReaderStatusFIFO->writeBlocking(&update, 1);

    CuePointer pN60dBSound = trackToOpen->findCueByType(mixxx::CueType::N60dBSound);
    if (pN60dBSound) {
        m_firstSoundFrameToVerify = pN60dBSound->getPosition();
    }

    DEBUG_ASSERT(!m_pChunkReadRequestFIFO->readAvailable());

    emit trackLoaded(
            trackToOpen,
            m_pAudioSource->getSignalInfo().getSampleRate(),
            m_pAudioSource->getSignalInfo().getChannelCount(),
            mixxx::audio::FramePos(m_pAudioSource->frameLength()));
}

void CachingReaderWorker::cleanupAllTrackFileCacheFiles(const QString& trackFileCacheDiskPath) {
    QMutexLocker locker(&s_trackFileCacheTracksMutex);

    // Clear all entries
    s_trackFileCacheTracks.clear();

    // Remove all TrackFileCache files from current session
    QDir tmpDir(trackFileCacheDiskPath);
    if (!tmpDir.exists()) {
        kLogger.warning()
                << "[TrackFileCache] -> Cleanup failed - directory doesn't exist:"
                << trackFileCacheDiskPath;
        return;
    }

    QStringList sessionFiles = tmpDir.entryList(QStringList() << gSessionPrefix + "*", QDir::Files);
    if (sDebugCachingReaderWorker) {
        kLogger.debug() << "[TrackFileCache] -> Found" << sessionFiles.size()
                        << "previous session files to clean up";
    }

    int removedCount = 0;
    int failedCount = 0;

    for (const QString& filename : std::as_const(sessionFiles)) {
        QString filePath = tmpDir.filePath(filename);
        QFile file(filePath);
        if (file.exists()) {
            if (file.remove()) {
                removedCount++;
                if (sDebugCachingReaderWorker) {
                    kLogger.debug() << "[TrackFileCache] -> Removed session "
                                       "TrackFileCache file:"
                                    << filePath;
                }
            } else {
                failedCount++;
                kLogger.warning() << "[TrackFileCache] -> Failed to remove previous "
                                     "session TrackFileCache file:"
                                  << filePath << "Error:" << file.errorString();
            }
        }
    }

    kLogger.info() << "[TrackFileCache] Cleanup completed - Removed:" << removedCount
                   << "files, Failed:" << failedCount << "files";
}

void CachingReaderWorker::unloadTrack() {
    closeAudioSource();

    const auto update = ReaderStatusUpdate::trackUnloaded();
    m_pReaderStatusFIFO->writeBlocking(&update, 1);
}

void CachingReaderWorker::quitWait() {
    m_stop = 1;
    m_semaRun.release();
    wait();
}

void CachingReaderWorker::verifyFirstSound(const CachingReaderChunk* pChunk,
        mixxx::audio::ChannelCount channelCount) {
    if (!m_firstSoundFrameToVerify.isValid()) {
        return;
    }

    const int firstSoundIndex =
            CachingReaderChunk::indexForFrame(static_cast<SINT>(
                    m_firstSoundFrameToVerify.toLowerFrameBoundary()
                            .value()));
    if (pChunk->getIndex() == firstSoundIndex) {
        mixxx::SampleBuffer sampleBuffer(kNumSoundFrameToVerify * channelCount);
        SINT end = static_cast<SINT>(m_firstSoundFrameToVerify.toLowerFrameBoundary().value());
        pChunk->readBufferedSampleFrames(sampleBuffer.data(),
                channelCount,
                mixxx::IndexRange::forward(end - 1, kNumSoundFrameToVerify));
        if (AnalyzerSilence::verifyFirstSound(sampleBuffer.span(),
                    mixxx::audio::FramePos(1),
                    channelCount)) {
            qDebug() << "First sound found at the previously stored position";
        } else {
            // This can happen in case of track edits or replacements, changed
            // encoders or encoding issues.
            qWarning() << "First sound has been moved! The beatgrid and "
                          "other annotations are no longer valid"
                       << m_pAudioSource->getUrlString();
        }
        m_firstSoundFrameToVerify = mixxx::audio::FramePos();
    }
}
