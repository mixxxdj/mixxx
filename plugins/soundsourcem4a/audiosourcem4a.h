#ifndef AUDIOSOURCEM4A_H
#define AUDIOSOURCEM4A_H

#include "sources/audiosource.h"
#include "samplebuffer.h"

#ifdef __MP4V2__
#include <mp4v2/mp4v2.h>
#else
#include <mp4.h>
#endif

#include <neaacdec.h>

#include <vector>

//As per QLibrary docs: http://doc.trolltech.com/4.6/qlibrary.html#resolve
#ifdef Q_OS_WIN
#define MY_EXPORT __declspec(dllexport)
#else
#define MY_EXPORT
#endif

namespace Mixxx {

class AudioSourceM4A: public AudioSource {
public:
    static AudioSourcePointer create(QUrl url);

    ~AudioSourceM4A();

    diff_type seekSampleFrame(diff_type frameIndex) /*override*/;

    size_type readSampleFrames(size_type numberOfFrames,
            CSAMPLE* sampleBuffer) /*override*/;

private:
    explicit AudioSourceM4A(QUrl url);

    Result postConstruct() /*override*/;

    void preDestroy();

    bool isValidSampleBlockId(MP4SampleId sampleBlockId) const;

    void restartDecoding(MP4SampleId sampleBlockId);

    MP4FileHandle m_hFile;
    MP4TrackId m_trackId;
    MP4SampleId m_maxSampleBlockId;
    MP4SampleId m_curSampleBlockId;

    typedef std::vector<u_int8_t> InputBuffer;
    InputBuffer m_inputBuffer;
    InputBuffer::size_type m_inputBufferOffset;
    InputBuffer::size_type m_inputBufferLength;

    NeAACDecHandle m_hDecoder;

    SampleBuffer m_decodeSampleBuffer;
    int m_decodeSampleBufferReadOffset;
    int m_decodeSampleBufferWriteOffset;

    diff_type m_curFrameIndex;
};

}

#endif
