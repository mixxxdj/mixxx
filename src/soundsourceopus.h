// soundsourceopus.cpp  -  Opus decoder
// Create by 14/01/2013 Tuukka Pasanen
// Based on work 2003 by Svein Magne Bang

#ifndef SOUNDSOURCEOPUS_H
#define SOUNDSOURCEOPUS_H

#include <QString>
#include <QtGlobal>
#define OV_EXCLUDE_STATIC_CALLBACKS
#include <opus/opusfile.h>

#include "soundsource.h"

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
};

#endif
