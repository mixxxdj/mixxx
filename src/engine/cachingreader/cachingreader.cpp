#include <QDir>
#include <QRegularExpression>
#include <QtDebug>

#include "moc_cachingreader.cpp"
#include "util/assert.h"
#include "util/compatibility/qatomic.h"
#include "util/counter.h"
#include "util/logger.h"
#include "util/sample.h"

namespace {

mixxx::Logger kLogger("CachingReader");
static QRegularExpression s_trailingSlashesRegex("/+$");

// This is the default hint frameCount that is adopted in case of Hint::kFrameCountForward and
// Hint::kFrameCountBackward count is provided. It matches 23 ms @ 44.1 kHz
// TODO() Do we suffer cache misses if we use an audio buffer of above 23 ms?
constexpr SINT kDefaultHintFrames = 1024;

// With CachingReaderChunk::kFrames = 8192 each chunk consumes
// 8192 frames * 2 channels/frame * 4-bytes per sample = 65 kB for stereo frame.
//
//     80 chunks ->  5120 KB =  5 MB
//
// Each deck (including sample decks) will use their own CachingReader.
// Consequently the total memory required for all allocated chunks depends
// on the number of decks. The amount of memory reserved for a single
// CachingReader must be multiplied by the number of decks to calculate
// the total amount!
//
// NOTE(uklotzde, 2019-09-05): Reduce this number to just few chunks
// (kNumberOfCachedChunksInMemory = 1, 2, 3, ...) for testing purposes
// to verify that the MRU/LRU cache works as expected. Even though
// massive drop outs are expected to occur Mixxx should run reliably!
constexpr SINT kNumberOfCachedChunksInMemory = 80;

} // anonymous namespace
CachingReader::RamPlayConfig CachingReader::s_ramPlayConfig;
QMutex CachingReader::s_configMutex;

CachingReader::CachingReader(const QString& group,
        UserSettingsPointer config,
        mixxx::audio::ChannelCount maxSupportedChannel)
        : m_pConfig(config),
          // Limit the number of in-flight requests to the worker. This should
          // prevent to overload the worker when it is not able to fetch those
          // requests from the FIFO timely. Otherwise outdated requests pile up
          // in the FIFO and it would take a long time to process them, just to
          // discard the results that most likely have already become obsolete.
          // TODO(XXX): Ideally the request FIFO would be implemented as a ring
          // buffer, where new requests replace old requests when full. Those
          // old requests need to be returned immediately to the CachingReader
          // that must take ownership and free them!!!
          m_chunkReadRequestFIFO(kNumberOfCachedChunksInMemory / 4),
          // The capacity of the back channel must be equal to the number of
          // allocated chunks, because the worker use writeBlocking(). Otherwise
          // the worker could get stuck in a hot loop!!!
          m_readerStatusUpdateFIFO(kNumberOfCachedChunksInMemory),
          m_state(STATE_IDLE),
          m_mruCachingReaderChunk(nullptr),
          m_lruCachingReaderChunk(nullptr),
          m_sampleBuffer(CachingReaderChunk::kFrames * maxSupportedChannel *
                  kNumberOfCachedChunksInMemory),
          m_worker(group,
                  &m_chunkReadRequestFIFO,
                  &m_readerStatusUpdateFIFO,
                  maxSupportedChannel) {
    // Initialize RAM play config (only once)
    initializeRamPlayConfig(m_pConfig);

    // Pass config values to worker
    QMutexLocker locker(&s_configMutex);
    m_worker.setRamPlayConfig(
            s_ramPlayConfig.enabled,
            s_ramPlayConfig.ramDiskPath,
            s_ramPlayConfig.maxSizeMB,
            s_ramPlayConfig.decksEnabled,
            s_ramPlayConfig.samplersEnabled,
            s_ramPlayConfig.previewEnabled);

    m_allocatedCachingReaderChunks.reserve(kNumberOfCachedChunksInMemory);
    // Divide up the allocated raw memory buffer into total_chunks
    // chunks. Initialize each chunk to hold nothing and add it to the free
    // list.
    for (SINT i = 0; i < kNumberOfCachedChunksInMemory; ++i) {
        CachingReaderChunkForOwner* c =
                new CachingReaderChunkForOwner(
                        mixxx::SampleBuffer::WritableSlice(
                                m_sampleBuffer,
                                CachingReaderChunk::kFrames * maxSupportedChannel * i,
                                CachingReaderChunk::kFrames * maxSupportedChannel));
        m_chunks.push_back(c);
        m_freeChunks.push_back(c);
    }

    // Forward signals from worker
    connect(&m_worker,
            &CachingReaderWorker::trackLoading,
            this,
            &CachingReader::trackLoading,
            Qt::DirectConnection);
    connect(&m_worker,
            &CachingReaderWorker::trackLoaded,
            this,
            &CachingReader::trackLoaded,
            Qt::DirectConnection);
    connect(&m_worker,
            &CachingReaderWorker::trackLoadFailed,
            this,
            &CachingReader::trackLoadFailed,
            Qt::DirectConnection);

    m_worker.start(QThread::HighPriority);
}

CachingReader::~CachingReader() {
    m_worker.quitWait();
    qDeleteAll(m_chunks);
}

void CachingReader::initializeRamPlayConfig(UserSettingsPointer pConfig) {
    QMutexLocker locker(&s_configMutex);

    if (s_ramPlayConfig.initialized) {
        return; // Already initialized
    }

    if (!pConfig) {
        // Set defaults
#ifdef Q_OS_WIN
        s_ramPlayConfig.ramDiskPath = "R:/MixxxTmp/";
#else
        s_ramPlayConfig.ramDiskPath = "/dev/shm/MixxxTmp/";
#endif
        s_ramPlayConfig.initialized = true;
        return;
    }

    // Check if config vars exist else create
    createRamPlayConfigVars(pConfig);

    s_ramPlayConfig.enabled = pConfig->getValue<bool>(ConfigKey("[RAM-Play]", "Enabled"));
    s_ramPlayConfig.maxSizeMB = pConfig->getValue<int>(ConfigKey("[RAM-Play]", "MaxSizeMB"));
    s_ramPlayConfig.decksEnabled = pConfig->getValue<bool>(ConfigKey("[RAM-Play]", "Decks"));
    s_ramPlayConfig.samplersEnabled = pConfig->getValue<bool>(ConfigKey("[RAM-Play]", "Samplers"));
    s_ramPlayConfig.previewEnabled =
            pConfig->getValue<bool>(ConfigKey("[RAM-Play]", "PreviewDeck"));

    QString dirName = pConfig->getValueString(ConfigKey("[RAM-Play]", "DirectoryName"));
    if (dirName.isEmpty()) {
        dirName = "MixxxTmp";
    }

#ifdef Q_OS_WIN
    QString driveLetter = pConfig->getValueString(ConfigKey("[RAM-Play]", "WindowsDrive"));
    driveLetter = driveLetter.replace(QRegularExpression("[^a-zA-Z]"), "").toUpper();
    if (driveLetter.isEmpty()) {
        driveLetter = "R";
    }
    s_ramPlayConfig.ramDiskPath = driveLetter + ":/" + dirName + "/";
#else
    QString basePath = pConfig->getValueString(ConfigKey("[RAM-Play]", "LinuxDrive"));
    if (basePath.isEmpty()) {
        basePath = "/dev/shm";
    }
    while (basePath.endsWith('/')) {
        basePath.chop(1);
    }
    s_ramPlayConfig.ramDiskPath = basePath + "/" + dirName + "/";
#endif

    s_ramPlayConfig.initialized = true;

    kLogger.debug() << "[RAM-PLAY] Config initialized: "
                    << "RAM-Play Enabled : " << s_ramPlayConfig.enabled
                    << "- Path:" << s_ramPlayConfig.ramDiskPath
                    << "- MaxSize:" << s_ramPlayConfig.maxSizeMB << "MB "
                    << "- Decks:" << s_ramPlayConfig.decksEnabled
                    << "- Samplers:" << s_ramPlayConfig.samplersEnabled
                    << "- PreviewDeck:" << s_ramPlayConfig.previewEnabled;
}

void CachingReader::createRamPlayConfigVars(UserSettingsPointer pConfig) {
    if (!pConfig) {
        return;
    }

    ConfigKey enabledKey("[RAM-Play]", "Enabled");
    if (!m_pConfig->exists(enabledKey)) {
        m_pConfig->setValue(enabledKey, false);
    }

    ConfigKey maxSizeKey("[RAM-Play]", "MaxSizeMB");
    if (!m_pConfig->exists(maxSizeKey)) {
        m_pConfig->setValue(maxSizeKey, 512);
    }

    ConfigKey dirNameKey("[RAM-Play]", "DirectoryName");
    if (!m_pConfig->exists(dirNameKey)) {
        m_pConfig->setValue(dirNameKey, QString("MixxxTmp"));
    }

    ConfigKey decksKey("[RAM-Play]", "Decks");
    if (!m_pConfig->exists(decksKey)) {
        m_pConfig->setValue(decksKey, true);
    }

    ConfigKey samplersKey("[RAM-Play]", "Samplers");
    if (!m_pConfig->exists(samplersKey)) {
        m_pConfig->setValue(samplersKey, true);
    }

    ConfigKey previewKey("[RAM-Play]", "PreviewDeck");
    if (!m_pConfig->exists(previewKey)) {
        m_pConfig->setValue(previewKey, false);
    }

#ifdef Q_OS_WIN
    ConfigKey driveKey("[RAM-Play]", "WindowsDrive");
    if (!m_pConfig->exists(driveKey)) {
        m_pConfig->setValue(driveKey, QString("R"));
    }
#else
    ConfigKey linuxDriveKey("[RAM-Play]", "LinuxDrive");
    if (!m_pConfig->exists(linuxDriveKey)) {
        m_pConfig->setValue(linuxDriveKey, QString("/dev/shm"));
    }
#endif
}

void CachingReader::freeChunkFromList(CachingReaderChunkForOwner* pChunk) {
    pChunk->removeFromList(
            &m_mruCachingReaderChunk,
            &m_lruCachingReaderChunk);
    pChunk->free();
    m_freeChunks.push_back(pChunk);
}

void CachingReader::freeChunk(CachingReaderChunkForOwner* pChunk) {
    DEBUG_ASSERT(pChunk);
    DEBUG_ASSERT(pChunk->getState() != CachingReaderChunkForOwner::READ_PENDING);

    const int removed = m_allocatedCachingReaderChunks.remove(pChunk->getIndex());
    Q_UNUSED(removed); // only used in DEBUG_ASSERT
    // We'll tolerate not being in allocatedCachingReaderChunks,
    // because sometime you free a chunk right after you allocated it.
    DEBUG_ASSERT(removed <= 1);

    freeChunkFromList(pChunk);
}

void CachingReader::freeAllChunks() {
    for (const auto& pChunk : std::as_const(m_chunks)) {
        // We will receive CHUNK_READ_INVALID for all pending chunk reads
        // which should free the chunks individually.
        if (pChunk->getState() == CachingReaderChunkForOwner::READ_PENDING) {
            continue;
        }

        if (pChunk->getState() != CachingReaderChunkForOwner::FREE) {
            freeChunkFromList(pChunk);
        }
    }
    DEBUG_ASSERT(!m_mruCachingReaderChunk);
    DEBUG_ASSERT(!m_lruCachingReaderChunk);

    m_allocatedCachingReaderChunks.clear();
}

CachingReaderChunkForOwner* CachingReader::allocateChunk(SINT chunkIndex) {
    if (m_freeChunks.empty()) {
        return nullptr;
    }
    CachingReaderChunkForOwner* pChunk = m_freeChunks.front();
    m_freeChunks.pop_front();

    pChunk->init(chunkIndex);

    m_allocatedCachingReaderChunks.insert(chunkIndex, pChunk);

    return pChunk;
}

CachingReaderChunkForOwner* CachingReader::allocateChunkExpireLRU(SINT chunkIndex) {
    auto* pChunk = allocateChunk(chunkIndex);
    if (!pChunk) {
        if (m_lruCachingReaderChunk) {
            freeChunk(m_lruCachingReaderChunk);
            pChunk = allocateChunk(chunkIndex);
        } else {
            kLogger.warning() << "No cached LRU chunk available for freeing";
        }
    }
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "allocateChunkExpireLRU" << chunkIndex << pChunk;
    }
    return pChunk;
}

CachingReaderChunkForOwner* CachingReader::lookupChunk(SINT chunkIndex) {
    // Defaults to nullptr if it's not in the hash.
    auto* pChunk = m_allocatedCachingReaderChunks.value(chunkIndex, nullptr);
    DEBUG_ASSERT(!pChunk || pChunk->getIndex() == chunkIndex);
    return pChunk;
}

void CachingReader::freshenChunk(CachingReaderChunkForOwner* pChunk) {
    DEBUG_ASSERT(pChunk);
    DEBUG_ASSERT(pChunk->getState() == CachingReaderChunkForOwner::READY);
    if (kLogger.traceEnabled()) {
        kLogger.trace()
                << "freshenChunk()"
                << pChunk->getIndex()
                << pChunk;
    }

    // Remove the chunk from the MRU/LRU list
    pChunk->removeFromList(
            &m_mruCachingReaderChunk,
            &m_lruCachingReaderChunk);

    // Reinsert has new head of MRU list
    pChunk->insertIntoListBefore(
            &m_mruCachingReaderChunk,
            &m_lruCachingReaderChunk,
            m_mruCachingReaderChunk);
}

CachingReaderChunkForOwner* CachingReader::lookupChunkAndFreshen(SINT chunkIndex) {
    auto* pChunk = lookupChunk(chunkIndex);
    if (pChunk && (pChunk->getState() == CachingReaderChunkForOwner::READY)) {
        freshenChunk(pChunk);
    }
    return pChunk;
}

// Invoked from the UI thread!!
#ifdef __STEM__
void CachingReader::newTrack(TrackPointer pTrack, mixxx::StemChannelSelection stemMask) {
#else
void CachingReader::newTrack(TrackPointer pTrack) {
#endif
    auto newState = pTrack ? STATE_TRACK_LOADING : STATE_TRACK_UNLOADING;
    auto oldState = m_state.fetchAndStoreAcquire(newState);

    // TODO():
    // BaseTrackPlayerImpl::slotLoadTrack() distributes the new track via
    // emit loadingTrack(pNewTrack, pOldTrack);
    // but the newTrack may change if we load a new track while the previous one
    // is still loading. This leads to inconsistent states for example a different
    // track in the Mixxx Title and the Deck label.
    if (oldState == STATE_TRACK_LOADING &&
            newState == STATE_TRACK_LOADING) {
        kLogger.warning()
                << "Loading a new track while loading a track may lead to inconsistent states";
    }
#ifdef __STEM__
    m_worker.newTrack(std::move(pTrack), stemMask);
#else
    m_worker.newTrack(std::move(pTrack));
#endif
}

// Called from the engine thread
void CachingReader::process() {
    ReaderStatusUpdate update;
    while (m_readerStatusUpdateFIFO.read(&update, 1) == 1) {
        auto* pChunk = update.takeFromWorker();
        if (pChunk) {
            // Result of a read request (with a chunk)
            DEBUG_ASSERT(atomicLoadRelaxed(m_state) != STATE_IDLE);
            DEBUG_ASSERT(
                    update.status == CHUNK_READ_SUCCESS ||
                    update.status == CHUNK_READ_EOF ||
                    update.status == CHUNK_READ_INVALID ||
                    update.status == CHUNK_READ_DISCARDED);
            if (m_state.loadAcquire() == STATE_TRACK_LOADING) {
                // Discard all results from pending read requests for the
                // previous track before the next track has been loaded.
                freeChunk(pChunk);
                continue;
            }
            DEBUG_ASSERT(atomicLoadRelaxed(m_state) == STATE_TRACK_LOADED);
            if (update.status == CHUNK_READ_SUCCESS) {
                // Insert or freshen the chunk in the MRU/LRU list after
                // obtaining ownership from the worker.
                freshenChunk(pChunk);
            } else {
                // Discard chunks that don't carry any data
                freeChunk(pChunk);
            }
            // Adjust the readable frame index range (if available)
            if (update.status != CHUNK_READ_DISCARDED) {
                m_readableFrameIndexRange = intersect(
                        m_readableFrameIndexRange,
                        update.readableFrameIndexRange());
            }
        } else {
            // State update (without a chunk)
            if (update.status == TRACK_LOADED) {
                // We have a new Track ready to go.
                // Assert that we either have had STATE_TRACK_LOADING before and all
                // chunks in the m_readerStatusUpdateFIFO have been discarded.
                // or the cache has been already cleared.
                // In case of two consecutive load events, we receive two consecutive
                // TRACK_LOADED without a chunk in between, assert this here.
                DEBUG_ASSERT(atomicLoadRelaxed(m_state) == STATE_TRACK_LOADING ||
                        (atomicLoadRelaxed(m_state) == STATE_TRACK_LOADED &&
                                !m_mruCachingReaderChunk && !m_lruCachingReaderChunk));
                // now purge also the recently used chunk list from the old track.
                if (m_mruCachingReaderChunk || m_lruCachingReaderChunk) {
                    DEBUG_ASSERT(atomicLoadRelaxed(m_state) == STATE_TRACK_LOADING);
                    freeAllChunks();
                }
                // Reset the readable frame index range
                m_readableFrameIndexRange = update.readableFrameIndexRange();
                m_state.storeRelease(STATE_TRACK_LOADED);
            } else {
                DEBUG_ASSERT(update.status == TRACK_UNLOADED);
                // This message could be processed later when a new
                // track is already loading! In this case the TRACK_LOADED will
                // be the very next status update.
                if (!m_state.testAndSetRelease(STATE_TRACK_UNLOADING, STATE_IDLE)) {
                    DEBUG_ASSERT(
                            atomicLoadRelaxed(m_state) == STATE_TRACK_LOADING ||
                            atomicLoadRelaxed(m_state) == STATE_IDLE);
                }
            }
        }
    }
}

CachingReader::ReadResult CachingReader::read(SINT startSample,
        SINT numSamples,
        bool reverse,
        CSAMPLE* buffer,
        mixxx::audio::ChannelCount channelCount) {
    // Check for bad inputs
    // Refuse to read from an invalid position
    VERIFY_OR_DEBUG_ASSERT(startSample % channelCount == 0) {
        kLogger.critical()
                << "Invalid arguments for read():"
                << "startSample =" << startSample;
        return ReadResult::UNAVAILABLE;
    }
    // Refuse to read from an invalid number of samples
    VERIFY_OR_DEBUG_ASSERT(numSamples % channelCount == 0) {
        kLogger.critical()
                << "Invalid arguments for read():"
                << "numSamples =" << numSamples;
        return ReadResult::UNAVAILABLE;
    }
    VERIFY_OR_DEBUG_ASSERT(numSamples >= 0) {
        kLogger.critical()
                << "Invalid arguments for read():"
                << "numSamples =" << numSamples;
        return ReadResult::UNAVAILABLE;
    }
    VERIFY_OR_DEBUG_ASSERT(buffer) {
        kLogger.critical()
                << "Invalid arguments for read():"
                << "buffer =" << buffer;
        return ReadResult::UNAVAILABLE;
    }

    // If no track is loaded, don't do anything.
    if (atomicLoadRelaxed(m_state) != STATE_TRACK_LOADED) {
        return ReadResult::UNAVAILABLE;
    }

    // If asked to read 0 samples, don't do anything. (this is a perfectly
    // reasonable request that happens sometimes.
    if (numSamples == 0) {
        return ReadResult::AVAILABLE; // nothing to do
    }

    // the samples are always read in forward direction
    // If reverse = true, the frames are copied in reverse order to the
    // destination buffer
    SINT sample = startSample;
    if (reverse) {
        // Start with the last sample in buffer
        sample -= numSamples;
    }

    SINT samplesRemaining = numSamples;

    // Process new messages from the reader thread before looking up
    // the first chunk and to update m_readableFrameIndexRange
    process();

    auto remainingFrameIndexRange =
            mixxx::IndexRange::forward(
                    CachingReaderChunk::samples2frames(sample, channelCount),
                    CachingReaderChunk::samples2frames(numSamples, channelCount));
    DEBUG_ASSERT(!remainingFrameIndexRange.empty());

    auto result = ReadResult::AVAILABLE;
    if (!intersect(remainingFrameIndexRange, m_readableFrameIndexRange).empty()) {
        // Fill the buffer up to the first readable sample with
        // silence. This may happen when the engine is in preroll,
        // i.e. if the frame index points a region before the first
        // track sample.
        if (remainingFrameIndexRange.start() < m_readableFrameIndexRange.start()) {
            const auto prerollFrameIndexRange =
                    mixxx::IndexRange::between(
                            remainingFrameIndexRange.start(),
                            m_readableFrameIndexRange.start());
            DEBUG_ASSERT(prerollFrameIndexRange.length() <= remainingFrameIndexRange.length());
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Preroll: Filling the first"
                        << prerollFrameIndexRange.length()
                        << "sample frames in"
                        << remainingFrameIndexRange
                        << "with silence. Audio signal starts at"
                        << m_readableFrameIndexRange.start();
            }
            const SINT prerollFrames = prerollFrameIndexRange.length();
            const SINT prerollSamples = CachingReaderChunk::frames2samples(
                    prerollFrames, channelCount);
            DEBUG_ASSERT(samplesRemaining >= prerollSamples);
            if (reverse) {
                SampleUtil::clear(&buffer[samplesRemaining - prerollSamples], prerollSamples);
            } else {
                SampleUtil::clear(buffer, prerollSamples);
                buffer += prerollSamples;
            }
            samplesRemaining -= prerollSamples;
            remainingFrameIndexRange.shrinkFront(prerollFrames);
            result = ReadResult::PARTIALLY_AVAILABLE;
        }

        // Read the actual samples from the audio source into the
        // buffer. The buffer will be filled with silence for every
        // unreadable sample or samples outside of the track region
        // later at the end of this function.
        if (!remainingFrameIndexRange.empty()) {
            // The intersection between the readable samples from the track
            // and the requested samples is not empty, so start reading.
            DEBUG_ASSERT(!intersect(remainingFrameIndexRange, m_readableFrameIndexRange).empty());
            DEBUG_ASSERT(remainingFrameIndexRange.start() >= m_readableFrameIndexRange.start());

            const SINT firstChunkIndex =
                    CachingReaderChunk::indexForFrame(remainingFrameIndexRange.start());
            SINT lastChunkIndex =
                    CachingReaderChunk::indexForFrame(remainingFrameIndexRange.end() - 1);
            for (SINT chunkIndex = firstChunkIndex;
                    chunkIndex <= lastChunkIndex;
                    ++chunkIndex) {
                // Process new messages from the reader thread before looking up
                // the next chunk
                process();

                // m_readableFrameIndexRange might change with every read operation!
                // On a cache miss audio data will be read from the audio source in
                // process() and the readable frame index range might get adjusted
                // if decoding errors occur.
                remainingFrameIndexRange =
                        intersect(
                                remainingFrameIndexRange,
                                m_readableFrameIndexRange);

                if (remainingFrameIndexRange.empty()) {
                    // No more readable data available. Exit the loop and
                    // fill the remaining buffer with silence.
                    kLogger.warning() << "Failed to read more sample data";
                    break;
                }
                lastChunkIndex =
                        CachingReaderChunk::indexForFrame(remainingFrameIndexRange.end() - 1);
                if (lastChunkIndex < chunkIndex) {
                    // No more readable data available. Exit the loop and
                    // fill the remaining buffer with silence.
                    kLogger.warning() << "Abort reading of sample data";
                    break;
                }

                mixxx::IndexRange bufferedFrameIndexRange;
                const CachingReaderChunkForOwner* const pChunk = lookupChunkAndFreshen(chunkIndex);
                if (pChunk && (pChunk->getState() == CachingReaderChunkForOwner::READY)) {
                    if (reverse) {
                        bufferedFrameIndexRange =
                                pChunk->readBufferedSampleFramesReverse(
                                        &buffer[samplesRemaining],
                                        channelCount,
                                        remainingFrameIndexRange);
                    } else {
                        bufferedFrameIndexRange =
                                pChunk->readBufferedSampleFrames(
                                        buffer,
                                        channelCount,
                                        remainingFrameIndexRange);
                    }
                } else {
                    // This will happen regularly when jumping to a new position
                    // within the file and decoding of the audio data is still
                    // pending.
                    DEBUG_ASSERT(!pChunk ||
                            (pChunk->getState() == CachingReaderChunkForOwner::READ_PENDING));
                    Counter("CachingReader::read(): Failed to read chunk on cache miss")++;
                    if (kLogger.traceEnabled()) {
                        kLogger.trace()
                                << "Cache miss for chunk with index"
                                << chunkIndex
                                << "- abort reading";
                    }
                    // Abort reading (see below)
                    DEBUG_ASSERT(bufferedFrameIndexRange.empty());
                }
                if (bufferedFrameIndexRange.empty()) {
                    if (samplesRemaining == numSamples) {
                        DEBUG_ASSERT(chunkIndex == firstChunkIndex);
                        // We have not read a single frame caused by a cache miss of
                        // the first required chunk. Inform the calling code that no
                        // data has been written into the buffer and to handle this
                        // situation appropriately.
                        return ReadResult::UNAVAILABLE;
                    }
                    // No more readable data available. Exit the loop and
                    // finally fill the remaining buffer with silence.
                    break;
                }
                DEBUG_ASSERT(bufferedFrameIndexRange.isSubrangeOf(remainingFrameIndexRange));
                if (remainingFrameIndexRange.start() < bufferedFrameIndexRange.start()) {
                    const auto paddingFrameIndexRange =
                            mixxx::IndexRange::between(
                                    remainingFrameIndexRange.start(),
                                    bufferedFrameIndexRange.start());
                    kLogger.warning()
                            << "Inserting"
                            << paddingFrameIndexRange.length()
                            << "frames of silence for unreadable audio data";
                    SINT paddingSamples = CachingReaderChunk::frames2samples(
                            paddingFrameIndexRange.length(), channelCount);
                    DEBUG_ASSERT(samplesRemaining >= paddingSamples);
                    if (reverse) {
                        SampleUtil::clear(&buffer[samplesRemaining - paddingSamples], paddingSamples);
                    } else {
                        SampleUtil::clear(buffer, paddingSamples);
                        buffer += paddingSamples;
                    }
                    samplesRemaining -= paddingSamples;
                    remainingFrameIndexRange.shrinkFront(paddingFrameIndexRange.length());
                    result = ReadResult::PARTIALLY_AVAILABLE;
                }
                const SINT chunkSamples = CachingReaderChunk::frames2samples(
                        bufferedFrameIndexRange.length(), channelCount);
                DEBUG_ASSERT(chunkSamples > 0);
                if (!reverse) {
                    buffer += chunkSamples;
                }
                DEBUG_ASSERT(samplesRemaining >= chunkSamples);
                samplesRemaining -= chunkSamples;
                remainingFrameIndexRange.shrinkFront(bufferedFrameIndexRange.length());
            }
        }
    }
    // Finally fill the remaining buffer with silence
    DEBUG_ASSERT(samplesRemaining >= 0);
    if (samplesRemaining > 0) {
        SampleUtil::clear(buffer, samplesRemaining);
        result = ReadResult::PARTIALLY_AVAILABLE;
    }
    return result;
}

void CachingReader::hintAndMaybeWake(const HintVector& hintList) {
    // If no file is loaded, skip.
    if (atomicLoadRelaxed(m_state) != STATE_TRACK_LOADED) {
        return;
    }

    // For every chunk that the hints indicated, check if it is in the cache. If
    // any are not, then wake.
    bool shouldWake = false;

    for (const auto& hint : hintList) {
        SINT hintFrame = hint.frame;
        SINT hintFrameCount = hint.frameCount;

        // Handle some special length values
        if (hintFrameCount == Hint::kFrameCountForward) {
            hintFrameCount = kDefaultHintFrames;
        } else if (hintFrameCount == Hint::kFrameCountBackward) {
            hintFrame -= kDefaultHintFrames;
            hintFrameCount = kDefaultHintFrames;
            if (hintFrame < 0) {
                hintFrameCount += hintFrame;
                if (hintFrameCount <= 0) {
                    continue;
                }
                hintFrame = 0;
            }
        }

        VERIFY_OR_DEBUG_ASSERT(hintFrameCount >= 0) {
            kLogger.warning() << "CachingReader: Ignoring negative hint length.";
            continue;
        }

        const auto readableFrameIndexRange = intersect(
                m_readableFrameIndexRange,
                mixxx::IndexRange::forward(hintFrame, hintFrameCount));
        if (readableFrameIndexRange.empty()) {
            continue;
        }

        const int firstChunkIndex = CachingReaderChunk::indexForFrame(readableFrameIndexRange.start());
        const int lastChunkIndex = CachingReaderChunk::indexForFrame(readableFrameIndexRange.end() - 1);
        for (int chunkIndex = firstChunkIndex; chunkIndex <= lastChunkIndex; ++chunkIndex) {
            CachingReaderChunkForOwner* pChunk = lookupChunk(chunkIndex);
            if (!pChunk) {
                shouldWake = true;
                pChunk = allocateChunkExpireLRU(chunkIndex);
                if (!pChunk) {
                    kLogger.warning()
                            << "Failed to allocate chunk"
                            << chunkIndex
                            << "for read request";
                    continue;
                }
                // Do not insert the allocated chunk into the MRU/LRU list,
                // because it will be handed over to the worker immediately
                CachingReaderChunkReadRequest request;
                request.giveToWorker(pChunk);
                if (kLogger.traceEnabled()) {
                    kLogger.trace()
                            << "Requesting read of chunk"
                            << request.chunk;
                }
                if (m_chunkReadRequestFIFO.write(&request, 1) != 1) {
                    kLogger.warning()
                            << "Failed to submit read request for chunk"
                            << chunkIndex;
                    // Revoke the chunk from the worker and free it
                    pChunk->takeFromWorker();
                    freeChunk(pChunk);
                }
            } else if (pChunk->getState() == CachingReaderChunkForOwner::READY) {
                // This will cause the chunk to be 'freshened' in the cache. The
                // chunk will be moved to the end of the LRU list.
                freshenChunk(pChunk);
            }
        }
    }

    // If there are chunks to be read, wake up.
    if (shouldWake) {
        m_worker.workReady();
    }
}
