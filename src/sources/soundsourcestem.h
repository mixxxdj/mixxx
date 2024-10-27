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

    void close() override;

  private:
    // Contains each stem source, or the main mix if opened in stereo mode
    std::vector<std::unique_ptr<SoundSourceSingleSTEM>> m_pStereoStreams;
    SampleBuffer m_buffer;

    mixxx::audio::ChannelCount m_requestedChannelCount;

  protected:
    OpenResult tryOpen(
            OpenMode mode,
            const OpenParams& params) override;

    ReadableSampleFrames readSampleFramesClamped(
            const WritableSampleFrames& sampleFrames) override;
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
