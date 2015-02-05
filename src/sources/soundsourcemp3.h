#ifndef SOUNDSOURCEMP3_H
#define SOUNDSOURCEMP3_H

#include "sources/soundsource.h"

class SoundSourceMp3: public Mixxx::SoundSource {
public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceMp3(QUrl url);

    Mixxx::AudioSourcePointer open() const /*override*/;
};

#endif
