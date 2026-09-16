#pragma once

#include <mutex>
#include <vector>

#include "audio/types.h"
#include "sources/soundsourceprovider.h"
#include "util/types.h"

namespace mixxx {

// Reads tracker/module files (MOD, S3M, XM, IT and many more) using
// libopenmpt. The whole module is rendered to an in-memory PCM buffer once
// while opening, which gives Mixxx sample-accurate random access and seeking
// just like any other source. The analyzer derives BPM/beatgrid from that PCM,
// so tempo sync works without any module-specific handling.
//
// Unlike the previous libmodplug implementation, every instance owns an
// independent decoder and its own sample buffer. There is no shared decoding
// state, so loading and analyzing multiple modules concurrently can no longer
// corrupt each other's audio/waveforms.
class SoundSourceOpenMPT final : public SoundSource {
  public:
    static constexpr auto kChannelCount = mixxx::audio::ChannelCount::stereo();
    // libopenmpt renders at any rate; 48 kHz is the library's recommended
    // output rate. Mixxx resamples to the engine rate afterwards.
    static constexpr auto kSampleRate = mixxx::audio::SampleRate(48000);

    // Render settings shared by all instances. They are applied to each
    // module on open, never to a shared/global decoder.
    struct Settings {
        // Interpolation filter taps: 1 = none/nearest, 2 = linear,
        // 4 = cubic, 8 = windowed sinc (libopenmpt default, highest quality).
        int interpolationFilterLength = 8;
        // Stereo separation in percent: 0 = mono, 100 = native, 200 = wide.
        int stereoSeparationPercent = 100;
    };

    // Store the decoding settings used for subsequently opened modules.
    static void configure(const Settings& settings);

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

    static std::mutex s_settingsMutex;
    static Settings s_settings;
    static Settings currentSettings();

    // Fully decoded, interleaved stereo PCM of the whole module.
    std::vector<CSAMPLE> m_sampleBuf;
};

class SoundSourceProviderOpenMPT : public SoundSourceProvider {
  public:
    static const QString kDisplayName;

    QString getDisplayName() const override {
        return kDisplayName;
    }

    QStringList getSupportedFileTypes() const override;

    SoundSourceProviderPriority getPriorityHint(
            const QString& supportedFileType) const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override {
        return newSoundSourceFromUrl<SoundSourceOpenMPT>(url);
    }
};

} // namespace mixxx
