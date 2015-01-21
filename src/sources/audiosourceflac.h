#ifndef AUDIOSOURCEFLAC_H
#define AUDIOSOURCEFLAC_H

#include "sources/audiosource.h"

#include <FLAC/stream_decoder.h>

#include <QFile>

#include <vector>

namespace Mixxx {

class AudioSourceFLAC: public AudioSource {
public:
    static AudioSourcePointer create(QUrl url);

    ~AudioSourceFLAC();

    diff_type seekSampleFrame(diff_type frameIndex) /*override*/;

    size_type readSampleFrames(size_type numberOfFrames,
            sample_type* sampleBuffer) /*override*/;
    size_type readSampleFramesStereo(size_type numberOfFrames,
            sample_type* sampleBuffer, size_type sampleBufferSize) /*override*/;

    // callback methods
    FLAC__StreamDecoderReadStatus flacRead(FLAC__byte buffer[], size_t *bytes);
    FLAC__StreamDecoderSeekStatus flacSeek(FLAC__uint64 offset);
    FLAC__StreamDecoderTellStatus flacTell(FLAC__uint64 *offset);
    FLAC__StreamDecoderLengthStatus flacLength(FLAC__uint64 *length);
    FLAC__bool flacEOF();
    FLAC__StreamDecoderWriteStatus flacWrite(const FLAC__Frame *frame,
            const FLAC__int32 * const buffer[]);
    void flacMetadata(const FLAC__StreamMetadata *metadata);
    void flacError(FLAC__StreamDecoderErrorStatus status);

private:
    explicit AudioSourceFLAC(QUrl url);

    Result postConstruct() /*override*/;

    void close();

    size_type readSampleFrames(size_type numberOfFrames,
            sample_type* sampleBuffer, size_type sampleBufferSize,
            bool readStereoSamples);

    QFile m_file;

    FLAC__StreamDecoder *m_decoder;
    // misc bits about the flac format:
    // flac encodes from and decodes to LPCM in blocks, each block is made up of
    // subblocks (one for each chan)
    // flac stores in 'frames', each of which has a header and a certain number
    // of subframes (one for each channel)
    size_type m_minBlocksize; // in time samples (audio samples = time samples * chanCount)
    size_type m_maxBlocksize;
    size_type m_minFramesize;
    size_type m_maxFramesize;
    sample_type m_sampleScale;

    typedef std::vector<sample_type> SampleBuffer;
    std::vector<sample_type> m_decodeSampleBuffer;
    SampleBuffer::size_type m_decodeSampleBufferReadOffset;
    SampleBuffer::size_type m_decodeSampleBufferWriteOffset;

    diff_type m_curFrameIndex;
};

}

#endif // ifndef AUDIOSOURCEFLAC_H
