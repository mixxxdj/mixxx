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
    /// // working
    std::vector<bool> m_needsResampling;
    SampleBuffer m_resampleInputBuffer;
    SampleBuffer m_resampleOutputBuffer;

    QMap<int, qint64> m_streamTotalFramesProcessed;
    QMap<int, qint64> m_streamTotalResamplingTime;
    // QElapsedTimer m_resampleTimer;
    // int m_debugCounter;

  protected:
    // Cubic interpolation function
    CSAMPLE cubicInterpolate(CSAMPLE y0, CSAMPLE y1, CSAMPLE y2, CSAMPLE y3, double mu);

    void initializeResamplers(int refSampleRate);
    // void processWithResampler(size_t streamIdx,
    //         const WritableSampleFrames& globalSampleFrames,
    //         CSAMPLE* pBuffer);
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

    void processWithResampler(size_t streamIdx,
            const WritableSampleFrames& globalSampleFrames,
            CSAMPLE* pBuffer);
    void interpolateAndMix(size_t streamIdx,
            SINT outputIndex,
            SINT sourceIndex,
            CSAMPLE fraction,
            CSAMPLE* pBuffer,
            std::size_t stemCount);
    void linearInterpolateAndMix(size_t streamIdx,
            SINT outputIndex,
            SINT sourceIndex,
            CSAMPLE fraction,
            CSAMPLE* pBuffer,
            std::size_t stemCount);
    void mixToOutput(size_t streamIdx,
            SINT outputIndex,
            CSAMPLE left,
            CSAMPLE right,
            CSAMPLE* pBuffer,
            std::size_t stemCount);
    CSAMPLE robustCubicInterpolate(CSAMPLE y0, CSAMPLE y1, CSAMPLE y2, CSAMPLE y3, CSAMPLE mu);
    CSAMPLE safeCubicInterpolate(CSAMPLE y0, CSAMPLE y1, CSAMPLE y2, CSAMPLE y3, CSAMPLE mu);
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
