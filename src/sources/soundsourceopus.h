#ifndef SOUNDSOURCEOPUS_H
#define SOUNDSOURCEOPUS_H

#include "sources/soundsource.h"

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

    Result parseMetadata(Mixxx::TrackMetadata* pMetadata) const /*override*/;
    QImage parseCoverArt() const /*override*/;

<<<<<<< HEAD
    OggOpusFile *m_pOggOpusFile;
>>>>>>> New SoundSource/AudioSource API
=======
    Mixxx::AudioSourcePointer open() const /*override*/;
>>>>>>> Split AudioSource from SoundSource
};

#endif
