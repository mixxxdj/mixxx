#ifndef SOUNDSOURCEFLAC_H
#define SOUNDSOURCEFLAC_H

#include "sources/soundsource.h"

class SoundSourceFLAC: public Mixxx::SoundSource {
public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceFLAC(QUrl url);

    Result parseMetadata(Mixxx::TrackMetadata* pMetadata) const /*override*/;
    QImage parseCoverArt() const /*override*/;

    Mixxx::AudioSourcePointer open() const /*override*/;
};

#endif // ifndef SOUNDSOURCEFLAC_H
