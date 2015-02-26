#ifndef AUDIOSOURCEMP3_H
#define AUDIOSOURCEMP3_H

#include "sources/audiosource.h"

#ifdef _MSC_VER
// So mad.h doesn't try to use inline assembly which MSVC doesn't support.
// Notably, FPM_64BIT does not require a 64-bit machine. It merely requires a
// compiler that supports 64-bit types.
#define FPM_64BIT
#endif
#include <mad.h>

#include <QFile>

#include <vector>

namespace Mixxx {

class AudioSourceMp3: public AudioSource {
public:
    static AudioSourcePointer create(QUrl url);

    ~AudioSourceMp3();

    SINT seekSampleFrame(SINT frameIndex) /*override*/;

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer) /*override*/;
    SINT readSampleFramesStereo(SINT numberOfFrames,
            CSAMPLE* sampleBuffer, SINT sampleBufferSize) /*override*/;

private:
    explicit AudioSourceMp3(QUrl url);

    Result postConstruct() /*override*/;

    void preDestroy();

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer, SINT sampleBufferSize,
            bool readStereoSamples);

    QFile m_file;
    quint64 m_fileSize;
    unsigned char* m_pFileData;

    mad_stream m_madStream;

    /** Struct used to store mad frames for seeking */
    struct SeekFrameType {
        SINT frameIndex;
        const unsigned char* pInputData;
    };

    /** It is not possible to make a precise seek in an mp3 file without decoding the whole stream.
     * To have precise seek within a limited range from the current decode position, we keep track
     * of past decoded frame, and their exact position. If a seek occurs and it is within the
     * range of frames we keep track of a precise seek occurs, otherwise an unprecise seek is performed
     */
    typedef std::vector<SeekFrameType> SeekFrameList;
    SeekFrameList m_seekFrameList; // ordered-by frameIndex
    SINT m_avgSeekFrameCount; // avg. sample frames per MP3 frame

    void addSeekFrame(SINT frameIndex, const unsigned char* pInputData);

    /** Returns the position in m_seekFrameList of the requested frame index. */
    SINT findSeekFrameIndex(SINT frameIndex) const;

    SINT m_curFrameIndex;

    SINT restartDecoding(const SeekFrameType& seekFrame);

    // current play position
    mad_frame m_madFrame;
    mad_synth m_madSynth;
    SINT m_madSynthCount; // left overs from the previous read
};

}

#endif
