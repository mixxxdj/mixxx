#pragma once

#include <QDateTime>
#include <QHash>
#include <QMutex>
#include <QString>
#include <QTemporaryFile>

#include "audio/frame.h"
#include "audio/types.h"
#include "engine/cachingreader/cachingreaderchunk.h"
#include "engine/engineworker.h"
#include "sources/audiosource.h"
#include "track/track_decl.h"

template<class DataType>
class FIFO;

struct RamTrackEntry;

// POD with trivial ctor/dtor/copy for passing through FIFO
typedef struct CachingReaderChunkReadRequest {
    CachingReaderChunk* chunk;

    void giveToWorker(CachingReaderChunkForOwner* chunkForOwner) {
        DEBUG_ASSERT(chunkForOwner);
        chunk = chunkForOwner;
        chunkForOwner->giveToWorker();
    }
} CachingReaderChunkReadRequest;

enum ReaderStatus {
    TRACK_LOADED,
    TRACK_UNLOADED,
    CHUNK_READ_SUCCESS,
    CHUNK_READ_EOF,
    CHUNK_READ_INVALID,
    CHUNK_READ_DISCARDED, // response without frame index range!
};

// POD with trivial ctor/dtor/copy for passing through FIFO
typedef struct ReaderStatusUpdate {
  private:
    CachingReaderChunk* chunk;
    SINT readableFrameIndexRangeStart;
    SINT readableFrameIndexRangeEnd;

  public:
    ReaderStatus status;

    void init(
            ReaderStatus statusArg,
            CachingReaderChunk* chunkArg,
            const mixxx::IndexRange& readableFrameIndexRangeArg) {
        status = statusArg;
        chunk = chunkArg;
        readableFrameIndexRangeStart = readableFrameIndexRangeArg.start();
        readableFrameIndexRangeEnd = readableFrameIndexRangeArg.end();
    }

    static ReaderStatusUpdate readDiscarded(
            CachingReaderChunk* chunk) {
        ReaderStatusUpdate update;
        update.init(CHUNK_READ_DISCARDED, chunk, mixxx::IndexRange());
        return update;
    }

    static ReaderStatusUpdate trackLoaded(
            const mixxx::IndexRange& readableFrameIndexRange) {
        DEBUG_ASSERT(!readableFrameIndexRange.empty());
        ReaderStatusUpdate update;
        update.init(TRACK_LOADED, nullptr, readableFrameIndexRange);
        return update;
    }

    static ReaderStatusUpdate trackUnloaded() {
        ReaderStatusUpdate update;
        update.init(TRACK_UNLOADED, nullptr, mixxx::IndexRange());
        return update;
    }

    CachingReaderChunkForOwner* takeFromWorker() {
        CachingReaderChunkForOwner* pChunk = nullptr;
        if (chunk) {
            DEBUG_ASSERT(dynamic_cast<CachingReaderChunkForOwner*>(chunk));
            pChunk = static_cast<CachingReaderChunkForOwner*>(chunk);
            chunk = nullptr;
            pChunk->takeFromWorker();
        }
        return pChunk;
    }

    mixxx::IndexRange readableFrameIndexRange() const {
        return mixxx::IndexRange::between(
                readableFrameIndexRangeStart,
                readableFrameIndexRangeEnd);
    }
} ReaderStatusUpdate;

class CachingReaderWorker : public EngineWorker {
    Q_OBJECT

  public:
    // Construct a CachingReader with the given group.
    CachingReaderWorker(const QString& group,
            FIFO<CachingReaderChunkReadRequest>* pChunkReadRequestFIFO,
            FIFO<ReaderStatusUpdate>* pReaderStatusFIFO,
            mixxx::audio::ChannelCount maxSupportedChannel);
    ~CachingReaderWorker() override = default;

    // Request to load a new track. wake() must be called afterwards.
#ifdef __STEM__
    void newTrack(TrackPointer pTrack, mixxx::StemChannelSelection stemMask);
#else
    void newTrack(TrackPointer pTrack);
#endif

    // Run upkeep operations like loading tracks and reading from file. Run by a
    // thread pool via the EngineWorkerScheduler.
    void run() override;

    void quitWait();
    static void cleanupSessionRamFiles();

    struct RamTrackEntry {
        QString group;    // the deck/sampler group using this track
        QString filePath; // the RAM file path

        // Optional: Add constructor for convenience
        RamTrackEntry() = default;
        RamTrackEntry(const QString& group, const QString& filePath)
                : group(group),
                  filePath(filePath) {
        }
    };
    void setRamPlayConfig(bool enabled, const QString& ramDiskPath);

  signals:
    // Emitted once a new track is loaded and ready to be read from.
    void trackLoading();
    void trackLoaded(TrackPointer pTrack,
            mixxx::audio::SampleRate sampleRate,
            mixxx::audio::ChannelCount channelCount,
            mixxx::audio::FramePos numFrame);
    void trackLoadFailed(TrackPointer pTrack, const QString& reason);

  private:
#ifdef __STEM__
    struct NewTrackRequest {
        TrackPointer track;
        mixxx::StemChannelSelection stemMask;
    };
#endif
    const QString m_group;
    QString m_tag;

    // Thread-safe FIFOs for communication between the engine callback and
    // reader thread.
    FIFO<CachingReaderChunkReadRequest>* m_pChunkReadRequestFIFO;
    FIFO<ReaderStatusUpdate>* m_pReaderStatusFIFO;

    // Queue of Tracks to load, and the corresponding lock. Must acquire the
    // lock to touch.
    QMutex m_newTrackMutex;
    QAtomicInt m_newTrackAvailable;
#ifdef __STEM__
    NewTrackRequest m_pNewTrack;
#else
    TrackPointer m_pNewTrack;
#endif

    void discardAllPendingRequests();

    /// call to be prepare for new tracks
    /// Make sure engine has been stopped before
    void closeAudioSource();

    /// Internal method to unload a track.
    /// does not emit signals
    void unloadTrack();

    /// Internal method to load a track. Emits trackLoaded when finished.
#ifdef __STEM__
    void loadTrack(const TrackPointer& pTrack, mixxx::StemChannelSelection stemMask);
#else
    void loadTrack(const TrackPointer& pTrack);
#endif
    // RAM-Play vars
    bool m_ramPlayEnabled;
    QString m_ramDiskPath;

    void openAudioSource(const TrackPointer& trackToOpen
#ifdef __STEM__
            ,
            mixxx::StemChannelSelection stemMask
#endif
    );

    ReaderStatusUpdate processReadRequest(
            const CachingReaderChunkReadRequest& request);

    void verifyFirstSound(const CachingReaderChunk* pChunk,
            mixxx::audio::ChannelCount channelCount);

    // The current audio source of the track loaded
    mixxx::AudioSourcePointer m_pAudioSource;

    mixxx::audio::FramePos m_firstSoundFrameToVerify;

    // Temporary buffer for reading samples from all channels
    // before conversion to a stereo signal.
    mixxx::SampleBuffer m_tempReadBuffer;

    // The maximum number of channel that this reader can support
    mixxx::audio::ChannelCount m_maxSupportedChannel;

    QAtomicInt m_stop;

    QTemporaryFile* m_tmpRamFile = nullptr;
    QString m_currentTrackURL;
    QSet<QString> m_ramFilesInUse;

    static QHash<QString, RamTrackEntry> s_ramTracks;
    static QMutex s_ramTracksMutex;
    static QString gSessionPrefix;

    // Add these private static helper methods
    static bool isRamFileUsedByOtherGroups(const QString& filePath, const QString& currentGroup);
    static void cleanupRamFileIfUnused(const QString& filePath);
    // static void updateRamTrackUsage(const QString& group, const QString& filePath);
    // static void removeRamTrackUsage(const QString& group);
    static void cleanupAllRamFiles();
    // QSet<QString> m_ramFilesInUse;
};
