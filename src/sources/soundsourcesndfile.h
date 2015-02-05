#ifndef SOUNDSOURCESNDFILE_H
#define SOUNDSOURCESNDFILE_H

#include "sources/soundsource.h"

class SoundSourceSndFile: public Mixxx::SoundSource {
public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceSndFile(QUrl url);

    Mixxx::AudioSourcePointer open() const /*override*/;
};

#endif
