#ifndef SOUNDSOURCEOPUS_H
#define SOUNDSOURCEOPUS_H

#include "sources/soundsource.h"

class SoundSourceOpus: public Mixxx::SoundSource {
public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceOpus(QUrl url);

    Result parseMetadata(Mixxx::TrackMetadata* pMetadata) const /*override*/;

    Mixxx::AudioSourcePointer open() const /*override*/;
};

#endif
