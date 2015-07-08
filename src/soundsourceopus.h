#ifndef SOUNDSOURCEOPUS_H
#define SOUNDSOURCEOPUS_H

#include "soundsource.h"

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <opus/opusfile.h>

class SoundSourceOpus: public Mixxx::SoundSource {
    typedef SoundSource Super;

<<<<<<< HEAD
class SoundSourceOpus : public Mixxx::SoundSource {
  public:
    explicit SoundSourceOpus(QString qFilename);
    virtual ~SoundSourceOpus();

    Result open();
    long seek(long);
    unsigned read(unsigned long size, const SAMPLE*);
    inline long unsigned length();
    Result parseHeader();
    QImage parseCoverArt();
    static QList<QString> supportedFileExtensions();

  private:
    OggOpusFile *m_ptrOpusFile;
    quint64 m_lFilelength;
=======
public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceOpus(QString qFilename);
    ~SoundSourceOpus();

    Result parseMetadata(Mixxx::TrackMetadata* pMetadata) /*override*/;
    QImage parseCoverArt() /*override*/;

    Result open() /*override*/;

    diff_type seekFrame(diff_type frameIndex);
    size_type readFrameSamplesInterleaved(size_type frameCount,
            sample_type* sampleBuffer) /*override*/;
    size_type readStereoFrameSamplesInterleaved(size_type frameCount,
            sample_type* sampleBuffer) /*override*/;

private:
    void close();

    OggOpusFile *m_pOggOpusFile;
>>>>>>> New SoundSource/AudioSource API
};

#endif
