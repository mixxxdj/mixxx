#ifndef SOUNDSOURCEMODPLUG_H
#define SOUNDSOURCEMODPLUG_H

#include "sources/soundsource.h"

// Class for reading tracker files using libmodplug.
// The whole file is decoded at once and saved
// in RAM to allow seeking and smooth operation in Mixxx.
class SoundSourceModPlug: public Mixxx::SoundSource {
public:
    static QList<QString> supportedFileExtensions();

    static QString getTypeFromUrl(QUrl url);

    explicit SoundSourceModPlug(QUrl url);

    Result parseMetadata(Mixxx::TrackMetadata* pMetadata) const /*override*/;

    QImage parseCoverArt() const /*override*/;

    Mixxx::AudioSourcePointer open() const /*override*/;
};

#endif
