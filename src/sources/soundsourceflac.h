#ifndef SOUNDSOURCEFLAC_H
#define SOUNDSOURCEFLAC_H

#include "sources/soundsource.h"

class SoundSourceFLAC: public Mixxx::SoundSource {
public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceFLAC(QUrl url);

    Mixxx::AudioSourcePointer open() const /*override*/;
};

#endif // ifndef SOUNDSOURCEFLAC_H
