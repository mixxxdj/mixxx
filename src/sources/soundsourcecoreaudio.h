#ifndef SOUNDSOURCECOREAUDIO_H
#define SOUNDSOURCECOREAUDIO_H

#include "sources/soundsource.h"

class SoundSourceCoreAudio : public Mixxx::SoundSource {
public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceCoreAudio(QUrl url);

    Mixxx::AudioSourcePointer open() const /*override*/;
};

#endif // ifndef SOUNDSOURCECOREAUDIO_H
