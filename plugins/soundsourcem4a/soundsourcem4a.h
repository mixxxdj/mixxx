#ifndef MIXXX_SOUNDSOURCEM4A_H
#define MIXXX_SOUNDSOURCEM4A_H

#include "sources/soundsourceplugin.h"

#include "singularsamplebuffer.h"

#ifdef __MP4V2__
#include <mp4v2/mp4v2.h>
#else
#include <mp4.h>
#endif

#include <neaacdec.h>

#include <vector>

namespace Mixxx {

class SoundSourceM4A: public SoundSourcePlugin {
public:
    explicit SoundSourceM4A(const QUrl& url);
    ~SoundSourceM4A();

    void close() override;

    SINT seekSampleFrame(SINT frameIndex) override;

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer) override;

private:
    Result tryOpen(const AudioSourceConfig& audioSrcCfg) override;

    bool isValidSampleBlockId(MP4SampleId sampleBlockId) const;

    void restartDecoding(MP4SampleId sampleBlockId);

    MP4FileHandle m_hFile;
    MP4TrackId m_trackId;
    MP4Duration m_framesPerSampleBlock;
    SINT m_numberOfPrefetchSampleBlocks;
    MP4SampleId m_maxSampleBlockId;
    MP4SampleId m_curSampleBlockId;

    typedef std::vector<u_int8_t> InputBuffer;
    InputBuffer m_inputBuffer;
    SINT m_inputBufferLength;
    SINT m_inputBufferOffset;

    NeAACDecHandle m_hDecoder;

    SingularSampleBuffer m_sampleBuffer;

    SINT m_curFrameIndex;
};

class SoundSourceProviderM4A: public SoundSourceProvider {
public:
    QString getName() const override;

    QStringList getSupportedFileExtensions() const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override;
};

} // namespace Mixxx

extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT
Mixxx::SoundSourceProviderPointer Mixxx_SoundSourcePluginAPI_getSoundSourceProvider();

#endif // MIXXX_SOUNDSOURCEM4A_H
