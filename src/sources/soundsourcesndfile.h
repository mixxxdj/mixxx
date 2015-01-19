#ifndef SOUNDSOURCESNDFILE_H
#define SOUNDSOURCESNDFILE_H

#include "sources/soundsource.h"

class SoundSourceSndFile: public Mixxx::SoundSource {
public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceSndFile(QUrl url);

    Result parseMetadata(Mixxx::TrackMetadata* pMetadata) const /*override*/;
    QImage parseCoverArt() const /*override*/;

    Mixxx::AudioSourcePointer open() const /*override*/;
};

#endif
