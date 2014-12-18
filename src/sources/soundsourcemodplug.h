#ifndef SOUNDSOURCEMODPLUG_H
#define SOUNDSOURCEMODPLUG_H

#include "sources/soundsource.h"

// Class for reading tracker files using libmodplug.
// The whole file is decoded at once and saved
// in RAM to allow seeking and smooth operation in Mixxx.
class SoundSourceModPlug: public Mixxx::SoundSource {
    typedef SoundSource Super;

public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceModPlug(QString fileName);

    Result parseMetadata(Mixxx::TrackMetadata* pMetadata) const /*override*/;

    QImage parseCoverArt() const /*override*/;

    Mixxx::AudioSourcePointer open() const /*override*/;
};

#endif
