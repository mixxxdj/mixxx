#ifndef MIXXX_SOUNDSOURCEFLAC_H
#define MIXXX_SOUNDSOURCEFLAC_H

#include "sources/soundsource.h"

#include "samplebuffer.h"

#include <FLAC/stream_decoder.h>

#include <QFile>

namespace Mixxx {

class SoundSourceFLAC: public SoundSource {
public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceFLAC(QUrl url);
    ~SoundSourceFLAC();

    void close() /*override*/;

    SINT seekSampleFrame(SINT frameIndex) /*override*/;

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer) /*override*/;
    SINT readSampleFramesStereo(SINT numberOfFrames,
            CSAMPLE* sampleBuffer, SINT sampleBufferSize) /*override*/;

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
    Result tryOpen(SINT channelCountHint) /*override*/;

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
    unsigned m_minBlocksize; // in time samples (audio samples = time samples * chanCount)
    unsigned m_maxBlocksize;
    unsigned m_minFramesize;
    unsigned m_maxFramesize;
    unsigned m_bitsPerSample;

    CSAMPLE m_sampleScaleFactor;

    SampleBuffer m_decodeSampleBuffer;
    SINT m_decodeSampleBufferLength;
    SINT m_decodeSampleBufferOffset;

    SINT m_curFrameIndex;
};

} // namespace Mixxx

#endif // MIXXX_SOUNDSOURCEFLAC_H
