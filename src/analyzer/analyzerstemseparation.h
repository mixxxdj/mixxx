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
// Execution provider policy (N22, CPU default):
//   Config:        [StemSeparation],execution_provider=cpu|openvino|auto
//   Env override:  $MIXXX_STEM_EXECUTION_PROVIDER (same values)
//   Default cpu keeps prior behavior. openvino/auto try the Intel iGPU EP
//   and fall back to CPU with a log telling how to install it:
//     sudo pacman -S openvino openvino-intel-gpu-plugin
//   CUDA/DirectML are not applicable on this Linux/iGPU target.
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

    /// N22: execution provider for ONNX inference. Reads
    /// [StemSeparation],execution_provider (cpu|openvino|auto, default cpu);
    /// $MIXXX_STEM_EXECUTION_PROVIDER overrides when set. Unknown/empty
    /// values fall back to cpu so the CPU default never breaks.
    static QString executionProvider(const UserSettingsPointer& pConfig);

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

    /// N18: optional overlap ratio for the Hann overlap-add in both offline
    /// paths. Reads [StemSeparation],overlap (0.5 or 0.25, default 0.25).
    /// Default 0.25 halves the chunk count (faster) and requires WOLA
    /// weight renormalization (see run paths), with small ripple at the
    /// track edges where coverage is partial; 0.5 keeps the legacy COLA
    /// path bit-identical (set manually for quality).
    static constexpr double kDefaultOverlap = 0.25;
    static double overlapRatio(const UserSettingsPointer& pConfig);
    static int hopSizeFor(int chunkSize, double overlap);

    /// N18: optional 3-stem mode. Reads [StemSeparation],stem_mode (4 or 3,
    /// default 3). Mode 3 folds bass+other into slot 3 ("Instruments") and
    /// silences slot 2 (bass); the 8-channel reader layout is untouched and
    /// the UI should hide the bass slot in mode 3 (documented, not enforced).
    /// Set stem_mode=4 manually for the legacy 4-stem layout.
    static constexpr int kDefaultStemMode = 3;
    static int stemMode(const UserSettingsPointer& pConfig);

    /// N18: folds 4 Mixxx-slot buffers into 3-stem mode in place:
    /// slot3 (other) += slot2 (bass), slot2 = silence. Slots 0 (vocals)
    /// and 1 (drums) untouched.
    static void foldTo3StemMode(QVector<float>* outStems[4]);

    /// N18: WOLA renormalization for non-COLA hops (overlap 0.25):
    /// divides each stem sample by the accumulated window weight, guarding
    /// with `eps` where coverage is zero. No-op when weights are ~1 (the
    /// default 0.5 path skips this to stay bit-identical).
    static void normalizeWola(
            QVector<float>* outStems[4], const QVector<float>& weight);

    /// Model-native sample rate (Hz). htdemucs was trained at 44100 Hz;
    /// inputs at any other rate must be resampled before inference and
    /// stems resampled back to the track-native rate before caching.
    /// Runs offline (AnalyzerThread / worker thread), never RT.
    static constexpr int kModelSampleRate = 44100;

    /// htdemucs ONNX emits stems in Demucs source order
    /// [drums, bass, other, vocals], while Mixxx slots
    /// (StemMp4Writer::Config::stems, SoundSourceSTEM 8-channel interleave
    /// [VL VR DL DR BL BR OL OR]) are [vocals, drums, bass, other].
    /// kOnnxStemForMixxxSlot[s] is the ONNX output index feeding Mixxx
    /// slot s; kMixxxSlotForOnnxStem[o] is the Mixxx slot fed by ONNX
    /// output o. The realtime path (EngineStemSeparator::readStem) already
    /// uses this order (vocals <- 3, percussion <- 0); both offline paths
    /// must route through it.
    static constexpr int kOnnxStemForMixxxSlot[4] = {3, 0, 1, 2};
    static constexpr int kMixxxSlotForOnnxStem[4] = {1, 2, 3, 0};

    /// Overlap-adds one inference chunk (`outFrames` stereo frames per stem,
    /// interleaved L,R) into the padded Mixxx-slot buffers, routing ONNX
    /// Demucs order [drums, bass, other, vocals] to Mixxx slots
    /// [vocals, drums, bass, other] via kMixxxSlotForOnnxStem. Shared by
    /// AnalyzerStemSeparation::runSeparationAndCache and
    /// OfflineSeparator::run so both offline paths route identically.
    /// Pure helper so tests can pin the routing without ONNX.
    static void accumulateChunk(float* srcStems[4],
            int outFrames,
            const QVector<float>& hannWindow,
            int startFrame,
            int paddedSamples,
            QVector<float>* outStems[4]);

    /// Linear-interpolated stereo resampler (interleaved L,R floats).
    /// No-op (returns `in`) when rates are equal/invalid or input empty.
    /// Pure helper so tests can verify 48000 <-> 44100 roundtrips.
    static QVector<float> resampleStereo(
            const QVector<float>& in, int srcRate, int dstRate);

    /// N21: chunk-streaming preview. After the first kPartialChunks chunks
    /// a playable .stem.mp4 prefix is written
    /// (StemCacheManager::partialStemFilePath) and exposed via
    /// StemCacheManager::markPartial() so playback can start while the
    /// remaining chunks keep processing in background with progress.
    /// At 50% overlap hop=N/2 ~= 171990 frames (~3.9 s @44100 Hz), so 3
    /// chunks cover ~11.7 s ("ej. 3 chunks ~12 s"); at the default 25%
    /// overlap the same 3 chunks cover ~17.5 s.
    static constexpr int kPartialChunks = 3;
    /// True exactly once per job: when chunkIndex is the kPartialChunks-th
    /// chunk (0-based) and more chunks remain.
    static bool shouldEmitPartial(int chunkIndex, int numChunks);
    /// Settled prefix length (inference-rate frames) covered after
    /// chunksDone chunks: chunksDone*hopSize clamped to [0, inferFrames].
    /// The tail needing future overlapping windows is excluded, so the
    /// preview boundary carries no half-window artifact.
    static int partialPrefixFrames(int chunksDone, int hopSize, int inferFrames);

  private:
    void runSeparationAndCache();

    UserSettingsPointer m_pConfig;
    mixxx::audio::ChannelCount m_channelCount;
    mixxx::audio::SampleRate m_sampleRate;
    QString m_location;
    StemCacheManager::CacheKey m_key;
    QVector<float> m_buffer; // interleaved stereo mix
    bool m_initialized = false;
    int m_stemMode = kDefaultStemMode;
    double m_overlap = kDefaultOverlap;
};
