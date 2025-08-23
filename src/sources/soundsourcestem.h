#pragma once

#include "sources/soundsourceffmpeg.h"
#include "sources/soundsourceprovider.h"
#include "util/samplebuffer.h"

namespace mixxx {

/// @brief Handle a single stem embedded in a stem file
class SoundSourceSingleSTEM : public SoundSourceFFmpeg {
  public:
    // streamIdx is the FFmpeg stream id, which may different than stemIdx + 1
    // because STEM may contain other non audio stream
    explicit SoundSourceSingleSTEM(const QUrl& url, unsigned int streamIdx);

    // SINT readSampleFramesResampled(
    //         const WritableSampleFrames& outFrames,
    //         SINT targetSampleRate);
  protected:
    OpenResult tryOpen(
            OpenMode mode,
            const OpenParams& params) override;

  private:
    unsigned int m_streamIdx;
};

/// @brief Handle a stem file, composed of multiple audio channel. Can open in
/// stereo or in stem (4 x stereo). Use OpenParams to request a maximum number of channels.
/// This allows decks which must not use STEM for performance or usability reason to use the
/// same soundsource.
class SoundSourceSTEM : public SoundSource {
  public:
    explicit SoundSourceSTEM(const QUrl& url);
    ~SoundSourceSTEM();

    void close() override;

  private:
    // Contains each stem source, or the main mix if opened in stereo mode
    std::vector<std::unique_ptr<SoundSourceSingleSTEM>> m_pStereoStreams;
    SampleBuffer m_buffer;

    mixxx::audio::ChannelCount m_requestedChannelCount;
    // QMap<int, SwrContext*> m_resamplers; // key: stream index
    // QMap<int, SampleBuffer> m_resampleBuffers;
    // QMap<int, SINT> m_resampleBufferPositions;
    // std::vector<SwrContext*> m_resamplers;
    // std::vector<SampleBuffer> m_resampleBuffers;
    // std::vector<SINT> m_resampleBufferPositions;
    // std::vector<bool> m_resamplerInitialized;

    // std::vector<SampleBuffer> m_resampledStems; // Pre-resampled stem data
    // std::vector<SINT> m_resampledStemPositions; // Current position in each
    // pre-resampled stem bool m_stemsResampled;                      // Flag
    // indicating if stems have been pre-resampled SINT m_totalFrames; // Total
    // frames in the premix

    // std::vector<bool> m_needsResampling;
    // std::vector<SwrContext*> m_resamplers;
    // SampleBuffer m_resampleInputBuffer;
    // SampleBuffer m_resampleOutputBuffer;
    // std::vector<SINT> m_resampleBufferPositions;

    // std::vector<SampleBuffer> m_resampleInputBuffers;
    // std::vector<SampleBuffer> m_resampleOutputBuffers;
    // std::vector<SINT> m_inputBufferPositions;
    // std::vector<SINT> m_outputBufferPositions;
    //   In your class header (soundsourcestem.h)

    /// // working
    std::vector<bool> m_needsResampling;
    SampleBuffer m_resampleInputBuffer;
    SampleBuffer m_resampleOutputBuffer;

    QMap<int, qint64> m_streamTotalFramesProcessed;
    QMap<int, qint64> m_streamTotalResamplingTime;
    QElapsedTimer m_resampleTimer;
    int m_debugCounter;

  protected:
    // Cubic interpolation function
    CSAMPLE cubicInterpolate(CSAMPLE y0, CSAMPLE y1, CSAMPLE y2, CSAMPLE y3, double mu);

    void initializeResamplers(int refSampleRate);
    void processWithResampler(size_t streamIdx,
            const WritableSampleFrames& globalSampleFrames,
            CSAMPLE* pBuffer);
    void processWithoutResampler(size_t streamIdx,
            const WritableSampleFrames& globalSampleFrames,
            CSAMPLE* pBuffer);
    void testCubicInterpolation();
    void showResamplingSummary();
    OpenResult tryOpen(
            OpenMode mode,
            const OpenParams& params) override;

    ReadableSampleFrames readSampleFramesClamped(
            const WritableSampleFrames& sampleFrames) override;
    // void preResampleStems();
    // void resampleEntireStem(size_t streamIdx, int targetSampleRate);
    // void processWithoutResampler(size_t streamIdx,
    //         const WritableSampleFrames& globalSampleFrames,
    //         CSAMPLE* pBuffer);

    // void initializeResamplers(int refSampleRate);
    // void processWithResampler(size_t streamIdx,
    //         const WritableSampleFrames& globalSampleFrames,
    //         CSAMPLE* pBuffer);
};

class SoundSourceProviderSTEM : public SoundSourceProvider {
  public:
    static const QString kDisplayName;

    ~SoundSourceProviderSTEM() override = default;

    QString getDisplayName() const override {
        return kDisplayName + QChar(' ') + getVersionString();
    }

    QStringList getSupportedFileTypes() const override;

    SoundSourceProviderPriority getPriorityHint(
            const QString& supportedFileType) const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override {
        return newSoundSourceFromUrl<SoundSourceSTEM>(url);
    }

    QString getVersionString() const;
};

} // namespace mixxx
