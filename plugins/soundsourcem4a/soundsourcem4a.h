#ifndef MIXXX_SOUNDSOURCEM4A_H
#define MIXXX_SOUNDSOURCEM4A_H

#include "sources/soundsourceplugin.h"
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

class SoundSourceM4A: public SoundSourcePlugin {
public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceM4A(QUrl url);
    ~SoundSourceM4A();

    void close() /*override*/;

    SINT seekSampleFrame(SINT frameIndex) /*override*/;

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer) /*override*/;

private:
    Result tryOpen(SINT channelCountHint) /*override*/;

    bool isValidSampleBlockId(MP4SampleId sampleBlockId) const;

    void restartDecoding(MP4SampleId sampleBlockId);

    MP4FileHandle m_hFile;
    MP4TrackId m_trackId;
    MP4SampleId m_maxSampleBlockId;
    MP4SampleId m_curSampleBlockId;

    typedef std::vector<u_int8_t> InputBuffer;
    InputBuffer m_inputBuffer;
    SINT m_inputBufferOffset;
    SINT m_inputBufferLength;

    NeAACDecHandle m_hDecoder;

    SampleBuffer m_sampleBuffer;
    int m_sampleBufferReadOffset;
    int m_sampleBufferWriteOffset;

    SINT m_curFrameIndex;
};

} // namespace Mixxx

extern "C" MY_EXPORT const char* getMixxxVersion();
extern "C" MY_EXPORT int getSoundSourceAPIVersion();
extern "C" MY_EXPORT Mixxx::SoundSource* getSoundSource(QString fileName);
extern "C" MY_EXPORT char** supportedFileExtensions();
extern "C" MY_EXPORT void freeFileExtensions(char** fileExtensions);

#endif // MIXXX_SOUNDSOURCEM4A_H
