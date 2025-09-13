#pragma once
#include <QAtomicInt>
#include <QHash>
#include <QList>
#include <QVarLengthArray>
#include <QVector>
#include <list>

#include "engine/cachingreader/cachingreaderworker.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"
#include "util/fifo.h"
#include "util/types.h"

// A Hint is an indication to the CachingReader that a certain section of a
// SoundSource will be used 'soon' and so it should be brought into memory by
// the reader work thread.
typedef struct Hint {
    enum class Type {
        SlipPosition,     // prio 1 (so far unused Mixxx 2.3 priority for reference)
        CurrentPosition,  // prio 1
        LoopStartEnabled, // prio 2
        MainCue,          // prio 10
        HotCue,           // prio 10
        LoopEndEnabled,   // prio 10
        LoopStart,        // prio 10
        FirstSound,
        IntroStart,
        IntroEnd,
        OutroStart
    };

    // The frame to ensure is present in memory.
    SINT frame;
    // If a range of frames should be present, use frameCount to indicate that the
    // range (frame, frame + frameCount) should be present in memory.
    SINT frameCount;
    // Currently unused -- but in the future could be used to prioritize certain
    // hints over others.
    Type type;

    // for the default frame count in forward direction
    static constexpr SINT kFrameCountForward = 0;
    static constexpr SINT kFrameCountBackward = -1;
} Hint;

// Note that we use a QVarLengthArray here instead of a QVector. Since this list
// is cleared on every callback and potentially referenced multiples times it's
// nicer to use a QVarLengthArray over a QVector because of two things:
//
// 1) No copy-on-write / implicit sharing behavior. If the reference count rises
//    above 1 then every non-const operation on a QVector clones it. We'd like
//    to avoid unnecessary memory allocation in the callback thread so this is
//    undesirable.
// 2) QVector::clear deletes the backing store (even if you call reserve) so we
//    reallocate on every callback. resize(0) should work but a future developer
//    may see a resize(0) and say "that's a silly way of writing clear()!" and
//    replace it without realizing.
typedef QVarLengthArray<Hint, 512> HintVector;

// CachingReader provides a layer on top of a SoundSource for reading samples
// from a file. Since we cannot do file I/O in the audio callback thread
// CachingReader and CachingReaderWorker (a worker thread) work in concert to
// read and decode relevant sections of a track in a background thread. The
// decoded chunks are kept in a cache by CachingReader with a
// least-recently-used (LRU) eviction policy. CachingReader exposes a method for
// indicating which chunks should be kept fresh in the cache (see
// hintAndMaybeWake). For example, the chunks around the playhead, the hotcue
// positions, and loop points are all portions of the track that the user is
// likely to dynamically jump to so we should keep them ready.
//
// The least recently used policy is implemented by keeping a linked list of the
// least recently used chunks. When a chunk is "freshened" (i.e. accessed via
// read or hinted via hintAndMaybeWake) then it is moved to the back of the
// least-recently-used list. When a chunk needs to be allocated and there are no
// free chunks then the least recently used chunk is free'd (see
// allocateChunkExpireLRU).
class CachingReader : public QObject {
    Q_OBJECT

  public:
    // Construct a CachingReader with the given group.
    CachingReader(const QString& group,
            UserSettingsPointer _config,
            mixxx::audio::ChannelCount maxSupportedChannel);
    ~CachingReader() override;

    void process();

    enum class ReadResult {
        // No samples read and buffer untouched(!), try again later in case of a cache miss
        UNAVAILABLE,
        // Some samples are missing and corresponding range in buffer has been cleared with silence
        PARTIALLY_AVAILABLE,
        // All requested samples are available and have been read into buffer
        AVAILABLE,
    };

    // Read numSamples from the SoundSource starting with sample into
    // buffer. It always writes numSamples to the buffer and otherwise
    // returns ReadResult::UNAVAILABLE.
    // It support reading stereo samples in reverse (backward) order.
    virtual ReadResult read(SINT startSample,
            SINT numSamples,
            bool reverse,
            CSAMPLE* buffer,
            mixxx::audio::ChannelCount channelCount);

    // Issue a list of hints, but check whether any of the hints request a chunk
    // that is not in the cache. If any hints do request a chunk not in cache,
    // then wake the reader so that it can process them. Must only be called
    // from the engine callback.
    void hintAndMaybeWake(const HintVector& hintList);

    // Request that the CachingReader load a new track. These requests are
    // processed in the work thread, so the reader must be woken up via wake()
    // for this to take effect.
#ifdef __STEM__
    void newTrack(TrackPointer pTrack, mixxx::StemChannelSelection stemMask = {});
#else
    void newTrack(TrackPointer pTrack);
#endif

    void setScheduler(EngineWorkerScheduler* pScheduler) {
        m_worker.setScheduler(pScheduler);
    }

  signals:
    // Emitted once a new track is loaded and ready to be read from.
    void trackLoading();
    void trackLoaded(TrackPointer pTrack,
            mixxx::audio::SampleRate trackSampleRate,
            mixxx::audio::ChannelCount trackChannelCount,
            mixxx::audio::FramePos trackNumFrame);
    void trackLoadFailed(TrackPointer pTrack, const QString& reason);

  private:
    const UserSettingsPointer m_pConfig;
    struct RamPlayConfig {
        bool enabled;
        QString ramDiskPath;
        int maxSizeMB;
        bool decksEnabled;
        bool samplersEnabled;
        bool previewEnabled;
        bool initialized;

        RamPlayConfig()
                : enabled(true),
                  maxSizeMB(512),
                  decksEnabled(true),
                  samplersEnabled(true),
                  previewEnabled(false),
                  initialized(false) {
        }
    };

    static RamPlayConfig s_ramPlayConfig;
    static QMutex s_configMutex;
    void initializeRamPlayConfig(UserSettingsPointer pConfig);
    void createRamPlayConfigVars(UserSettingsPointer pConfig);

    // Thread-safe FIFOs for communication between the engine callback and
    // reader thread.
    FIFO<CachingReaderChunkReadRequest> m_chunkReadRequestFIFO;
    FIFO<ReaderStatusUpdate> m_readerStatusUpdateFIFO;

    // Looks for the provided chunk number in the index of in-memory chunks and
    // returns it if it is present. If not, returns nullptr. If it is present then
    // freshenChunk is called on the chunk to make it the MRU chunk.
    CachingReaderChunkForOwner* lookupChunkAndFreshen(SINT chunkIndex);

    // Looks for the provided chunk number in the index of in-memory chunks and
    // returns it if it is present. If not, returns nullptr.
    CachingReaderChunkForOwner* lookupChunk(SINT chunkIndex);

    // Moves the provided chunk to the MRU position.
    void freshenChunk(CachingReaderChunkForOwner* pChunk);

    // Returns a CachingReaderChunk to the free list
    void freeChunk(CachingReaderChunkForOwner* pChunk);
    void freeChunkFromList(CachingReaderChunkForOwner* pChunk);

    // Returns all allocated chunks to the free list
    void freeAllChunks();

    // Gets a chunk from the free list. Returns nullptr if none available.
    CachingReaderChunkForOwner* allocateChunk(SINT chunkIndex);

    // Gets a chunk from the free list, frees the LRU CachingReaderChunk if none available.
    CachingReaderChunkForOwner* allocateChunkExpireLRU(SINT chunkIndex);

    enum State {
        STATE_IDLE,
        STATE_TRACK_LOADING,
        STATE_TRACK_UNLOADING,
        STATE_TRACK_LOADED,
    };
    QAtomicInt m_state;

    // Keeps track of all CachingReaderChunks we've allocated.
    QVector<CachingReaderChunkForOwner*> m_chunks;

    // List of free chunks. Linked list so that we have constant time insertions
    // and deletions. Iteration is not necessary.
    std::list<CachingReaderChunkForOwner*> m_freeChunks;

    // Keeps track of what CachingReaderChunks we've allocated and indexes them based on what
    // chunk number they are allocated to.
    QHash<int, CachingReaderChunkForOwner*> m_allocatedCachingReaderChunks;

    // The linked list of recently-used chunks.
    CachingReaderChunkForOwner* m_mruCachingReaderChunk;
    CachingReaderChunkForOwner* m_lruCachingReaderChunk;

    // The raw memory buffer which is divided up into chunks.
    mixxx::SampleBuffer m_sampleBuffer;

    // The readable frame index range as reported by the worker.
    mixxx::IndexRange m_readableFrameIndexRange;

    CachingReaderWorker m_worker;
};
