#pragma once

#include "sources/soundsourceprovider.h"
#include "util/samplebuffer.h"

namespace mixxx {

class SoundSourceFFmpeg;

/// @brief Handle a stem file, composed of multiple audio channel. Can open in
/// stereo or in stem (4 x stereo). Use OpenParams to request a maximum number of channels.
/// This allows decks which must not use STEM for performance or usability reason to use the
/// same soundsource.
///
/// Native layout (see StemMp4Writer::Config / writeMp4 in
/// src/engine/stems/stemmp4writer.cpp): the MP4 holds 5 stereo audio
/// streams. Stream 0 is the premixed main mix and is NEVER decoded by this
/// class. Streams 1..4 are the individual stems in file order (for
/// writer-generated files: vocals, drums, bass, other). In 8-channel mode
/// the decoded frames are interleaved as [VL VR DL DR BL BR OL OR]
/// (each stem contributes its stereo L/R pair at 2*stemIdx). Every audio stream must be stereo
/// (nb_channels == 2) and share one sample rate and codec, otherwise
/// opening fails cleanly with OpenResult::Failed (no crash, no partial state).
class SoundSourceSTEM : public SoundSource {
  public:
    explicit SoundSourceSTEM(const QUrl& url);
    ~SoundSourceSTEM() override;

    void close() override;

  private:
    // Contains each stem source, or the main mix if opened in stereo mode
    std::vector<std::unique_ptr<SoundSourceFFmpeg>> m_pStereoStreams;
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
