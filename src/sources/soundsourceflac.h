#ifndef MIXXX_SOUNDSOURCEFLAC_H
#define MIXXX_SOUNDSOURCEFLAC_H

#include "sources/soundsourceprovider.h"

#include "util/circularsamplebuffer.h"

#include <FLAC/stream_decoder.h>

#include <QFile>

namespace mixxx {

class SoundSourceFLAC: public SoundSource {
public:
    explicit SoundSourceFLAC(const QUrl& url);
    ~SoundSourceFLAC() override;

    void close() override;

    SINT seekSampleFrame(SINT frameIndex) override;

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer) override;
    SINT readSampleFramesStereo(SINT numberOfFrames,
            CSAMPLE* sampleBuffer, SINT sampleBufferSize) override;

    // callback methods
    FLAC__StreamDecoderReadStatus flacRead(FLAC__byte buffer[], size_t* bytes);
    FLAC__StreamDecoderSeekStatus flacSeek(FLAC__uint64 offset);
    FLAC__StreamDecoderTellStatus flacTell(FLAC__uint64* offset);
    FLAC__StreamDecoderLengthStatus flacLength(FLAC__uint64* length);
    FLAC__bool flacEOF();
    FLAC__StreamDecoderWriteStatus flacWrite(const FLAC__Frame *frame,
            const FLAC__int32* const buffer[]);
    void flacMetadata(const FLAC__StreamMetadata* metadata);
    void flacError(FLAC__StreamDecoderErrorStatus status);

private:
    OpenResult tryOpen(const AudioSourceConfig& audioSrcCfg) override;

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer, SINT sampleBufferSize,
            bool readStereoSamples);

    QFile m_file;

    FLAC__StreamDecoder *m_decoder;
    // misc bits about the flac format:
    // flac encodes from and decodes to LPCM in blocks, each block is made up of
    // subblocks (one for each chan)
    // flac stores in 'frames', each of which has a header and a certain number
    // of subframes (one for each channel)
    SINT m_maxBlocksize; // in time samples (audio samples = time samples * chanCount)
    SINT m_bitsPerSample;

    CSAMPLE m_sampleScaleFactor;

    SingularSampleBuffer m_sampleBuffer;

    SINT m_curFrameIndex;
};

class SoundSourceProviderFLAC: public SoundSourceProvider {
public:
    QString getName() const override;

    QStringList getSupportedFileExtensions() const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override {
        return newSoundSourceFromUrl<SoundSourceFLAC>(url);
    }
};

} // namespace mixxx

#endif // MIXXX_SOUNDSOURCEFLAC_H
