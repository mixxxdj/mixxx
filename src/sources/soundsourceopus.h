#ifndef SOUNDSOURCEOPUS_H
#define SOUNDSOURCEOPUS_H

#include "sources/soundsource.h"

class SoundSourceOpus: public Mixxx::SoundSource {
    typedef SoundSource Super;

public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceOpus(QString qFilename);

    Result parseMetadata(Mixxx::TrackMetadata* pMetadata) const /*override*/;
    QImage parseCoverArt() const /*override*/;

    Mixxx::AudioSourcePointer open() const /*override*/;
};

#endif
