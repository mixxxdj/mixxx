// TODO(XXX): This implementation has been copied from the
// original file soundsourcemediafoundation.h and needs
// a major refactoring! Please refer to the helpful comments
// of daschuer in the following pull requests:
// https://github.com/mixxxdj/mixxx/pull/411

#ifndef AUDIOSOURCEMEDIAFOUNDATIONMEDIAFOUNDATION_H
#define AUDIOSOURCEMEDIAFOUNDATIONMEDIAFOUNDATION_H

#include "sources/audiosource.h"

#include <windows.h>

#ifdef Q_OS_WIN
#define MY_EXPORT __declspec(dllexport)
#else
#define MY_EXPORT
#endif

class IMFSourceReader;
class IMFMediaType;
class IMFMediaSource;

namespace Mixxx {

class AudioSourceMediaFoundation : public AudioSource {
public:
    static AudioSourcePointer create(QUrl url);

    ~AudioSourceMediaFoundation();

    diff_type seekSampleFrame(diff_type frameIndex) /*override*/;

    size_type readSampleFrames(size_type numberOfFrames, sample_type* sampleBuffer) /*override*/;

private:
    explicit AudioSourceMediaFoundation(QUrl url);

    Result postConstruct() /*override*/;

    void close();

    bool configureAudioStream();

    void copyFrames(sample_type *dest, size_t *destFrames, const sample_type *src,
            size_t srcFrames);

    HRESULT m_hrCoInitialize;
    HRESULT m_hrMFStartup;
    IMFSourceReader *m_pReader;
    IMFMediaType *m_pAudioType;
    wchar_t *m_wcFilename;
    int m_nextFrame;
    sample_type *m_leftoverBuffer;
    size_t m_leftoverBufferSize;
    size_t m_leftoverBufferLength;
    int m_leftoverBufferPosition;
    qint64 m_mfDuration;
    long m_iCurrentPosition;
    bool m_dead;
    bool m_seeking;
};

}

#endif // ifndef AUDIOSOURCEMEDIAFOUNDATIONMEDIAFOUNDATION_H
