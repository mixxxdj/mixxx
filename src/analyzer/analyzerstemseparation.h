#pragma once

// AnalyzerStemSeparation — offline AI stem separation as an Analyzer.
//
// Runs inside AnalyzerThread (never on the real-time audio thread). It
// buffers the stereo samples that AnalyzerThread already decoded via
// SoundSourceProxy, then in storeResults() runs the Hann 50% overlap-add
// separation (same algorithm as OfflineSeparator::run(), see
// src/engine/stems/offlineseparator.cpp) and writes a native
// {hash}.stem.mp4 into the stem cache.
//
// Model policy (never assume a model exists):
//   Expected path: /usr/local/share/stem-models/htdemucs_fp16weights.onnx
//   Override:      $MIXXX_STEM_MODEL
//   If the model is missing/unloadable or inference fails at any chunk,
//   the analyzer falls back to passthrough stems (vocals = mix,
//   drums/bass/other = silence) so analysis always succeeds.
//
// Cache policy:
//   key = SHA256(path + mtime + size)  (StemCacheManager::generateKey)
//   file = <cacheDir>/<key>/<key>.stem.mp4, where cacheDir() is
//          ~/.local/share/mixxx/analysis/stems
//   Second load of the same track hits the cache (hasStems() == true) and
//   performs no inference. tryMarkProcessing() prevents duplicate work when
//   two AnalyzerThreads (or EngineDeck) race on the same track.

#include <QVector>

#include "analyzer/analyzer.h"
#include "engine/stems/stemcachemanager.h"
#include "preferences/usersettings.h"

class AnalyzerStemSeparation : public Analyzer {
  public:
    explicit AnalyzerStemSeparation(UserSettingsPointer pConfig);
    ~AnalyzerStemSeparation() override = default;

    bool initialize(const AnalyzerTrack& track,
            mixxx::audio::SampleRate sampleRate,
            mixxx::audio::ChannelCount channelCount,
            SINT frameLength) override;
    bool processSamples(const CSAMPLE* pIn, SINT count) override;
    void storeResults(TrackPointer pTrack) override;
    void cleanup() override;

    // ---- Static helpers (also used by unit tests) ------------------------

    /// Default model path documented for offline separation. Never assumed
    /// to exist; callers must handle a missing file via passthrough.
    static QString defaultModelPath();

    /// Effective model path: $MIXXX_STEM_MODEL if set, else defaultModelPath().
    static QString effectiveModelPath(const QString& overridePath = {});

    /// Hann window with periodic=false (denominator N-1), identical to the
    /// one used in OfflineSeparator::run(). At 50% overlap the windows sum
    /// to ~1.0 (up to the 1/(N-1) vs 1/N convention error).
    static QVector<float> makeHannWindow(int size);

    /// 50% overlap-add of per-chunk stem outputs into `dst` (interleaved
    /// stereo). `srcChunks[chunk][stem]` holds kChunkSize*2 floats each.
    /// Pure helper so tests can verify reconstruction ~= 1.0 without ONNX.
    static void overlapAdd(const QVector<QVector<QVector<float>>>& srcChunks,
            int chunkSize,
            int totalFrames,
            const QVector<float>& hannWindow,
            QVector<float> outStems[4]);

    /// Passthrough fallback: vocals = mix, rest = silence.
    static void passthrough(const QVector<float>& mix,
            QVector<float>* outStems[4]);

    /// Whether offline stem analysis is enabled in the config.
    /// Default true; set [StemSeparation],offline_enabled=0 to disable.
    static bool isEnabled(const UserSettingsPointer& pConfig);

  private:
    void runSeparationAndCache();

    UserSettingsPointer m_pConfig;
    mixxx::audio::ChannelCount m_channelCount;
    mixxx::audio::SampleRate m_sampleRate;
    QString m_location;
    StemCacheManager::CacheKey m_key;
    QVector<float> m_buffer; // interleaved stereo mix
    bool m_initialized = false;
};
