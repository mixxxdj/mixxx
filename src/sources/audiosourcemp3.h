#ifndef AUDIOSOURCEMP3_H
#define AUDIOSOURCEMP3_H

#include "sources/audiosource.h"
#include "util/defs.h"

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
    static AudioSourcePointer create(QString fileName);

    ~AudioSourceMp3();

    diff_type seekSampleFrame(diff_type frameIndex) /*override*/;

    size_type readSampleFrames(size_type numberOfFrames,
            sample_type* sampleBuffer) /*override*/;
    size_type readSampleFramesStereo(size_type numberOfFrames,
            sample_type* sampleBuffer, size_type sampleBufferSize) /*override*/;

private:
    explicit AudioSourceMp3(QString fileName);

    Result open();

    void close();

    inline size_type skipFrameSamples(size_type numberOfFrames) {
        return readSampleFrames(numberOfFrames, NULL);
    }
    size_type readSampleFrames(size_type numberOfFrames,
            sample_type* sampleBuffer, size_type sampleBufferSize,
            bool readStereoSamples);

    QFile m_file;
    quint64 m_fileSize;
    unsigned char* m_pFileData;

    mad_stream m_madStream;

    /** Struct used to store mad frames for seeking */
    struct SeekFrameType {
        diff_type frameIndex;
        const unsigned char* pInputData;
    };

    /** It is not possible to make a precise seek in an mp3 file without decoding the whole stream.
     * To have precise seek within a limited range from the current decode position, we keep track
     * of past decoded frame, and their exact position. If a seek occurs and it is within the
     * range of frames we keep track of a precise seek occurs, otherwise an unprecise seek is performed
     */
    typedef std::vector<SeekFrameType> SeekFrameList;
    SeekFrameList m_seekFrameList; // ordered-by frameIndex
    size_type m_avgSeekFrameCount; // avg. sample frames per MP3 frame

    void addSeekFrame(diff_type frameIndex, const unsigned char* pInputData);

    /** Returns the position in m_seekFrameList of the requested frame index. */
    SeekFrameList::size_type findSeekFrameIndex(diff_type frameIndex) const;

    diff_type m_curFrameIndex;

    diff_type restartDecoding(const SeekFrameType& seekFrame);

    // current play position
    mad_frame m_madFrame;
    mad_synth m_madSynth;
    size_type m_madSynthCount; // left overs from the previous read
};

}

#endif
