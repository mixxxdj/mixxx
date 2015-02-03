#ifndef SOUNDSOURCEFFMPEG_H
#define SOUNDSOURCEFFMPEG_H

#include "sources/soundsource.h"

class SoundSourceFFmpeg : public Mixxx::SoundSource {
public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceFFmpeg(QUrl url);

    Result parseMetadata(Mixxx::TrackMetadata* pMetadata) const /*override*/;
    QImage parseCoverArt() const /*override*/;

    Mixxx::AudioSourcePointer open() const /*override*/;
};

#endif
