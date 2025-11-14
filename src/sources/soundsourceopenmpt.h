#pragma once

#include <memory>
#include <vector>

#include "sources/soundsourceprovider.h"
#include "sources/trackerdsp.h"

// forward declarations for openmpt types
namespace openmpt {
class module;
}

namespace mixxx {

/// the whole file is decoded at once and saved in RAM
/// to allow seeking and smooth operation in mixxx
class SoundSourceOpenMPT final : public SoundSource {
  public:
    static constexpr auto kChannelCount = mixxx::audio::ChannelCount::stereo();
    static constexpr auto kSampleRate = mixxx::audio::SampleRate(44100);

    // apply settings for decoding
    static void configure(
            unsigned int bufferSizeLimit,
            const TrackerDSP::Settings& dspSettings);

    explicit SoundSourceOpenMPT(const QUrl& url);
    ~SoundSourceOpenMPT() override;

    std::pair<ImportResult, QDateTime> importTrackMetadataAndCoverImage(
            TrackMetadata* pTrackMetadata,
            QImage* pCoverArt,
            bool resetMissingTagMetadata) const override;

    void close() override;

  protected:
    ReadableSampleFrames readSampleFramesClamped(
            const WritableSampleFrames& sampleFrames) override;

  private:
    OpenResult tryOpen(
            OpenMode mode,
            const OpenParams& params) override;

    static unsigned int s_bufferSizeLimit;
    static TrackerDSP::Settings s_dspSettings;

    std::unique_ptr<openmpt::module> m_pModule;
    QByteArray m_fileBuf;

    typedef std::vector<CSAMPLE> ModSampleBuffer;
    ModSampleBuffer m_sampleBuf;

    TrackerDSP m_dsp;
};

class SoundSourceProviderOpenMPT : public SoundSourceProvider {
  public:
    static const QString kDisplayName;

    QString getDisplayName() const override {
        return kDisplayName;
    }

    QStringList getSupportedFileTypes() const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override {
        return newSoundSourceFromUrl<SoundSourceOpenMPT>(url);
    }
};

} // namespace mixxx
