#ifndef SOUNDSOURCEMP3_H
#define SOUNDSOURCEMP3_H

#include "sources/soundsource.h"

/**
 *@author Tue and Ken Haste Andersen
 */

class SoundSourceMp3: public Mixxx::SoundSource {
public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceMp3(QUrl url);

    Result parseMetadata(Mixxx::TrackMetadata* pMetadata) const /*override*/;
    QImage parseCoverArt() const /*override*/;

    Mixxx::AudioSourcePointer open() const /*override*/;
};

#endif
