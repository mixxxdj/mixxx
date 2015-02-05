#ifndef SOUNDSOURCEOGGVORBIS_H
#define SOUNDSOURCEOGGVORBIS_H

#include "sources/soundsource.h"

class SoundSourceOggVorbis: public Mixxx::SoundSource {
public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceOggVorbis(QUrl url);

    Mixxx::AudioSourcePointer open() const /*override*/;
};

#endif
