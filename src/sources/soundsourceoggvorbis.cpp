#include "sources/soundsourceoggvorbis.h"

#include "sources/audiosourceoggvorbis.h"

QList<QString> SoundSourceOggVorbis::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("ogg");
    return list;
}

SoundSourceOggVorbis::SoundSourceOggVorbis(QUrl url) :
        SoundSource(url, "ogg") {
}

Mixxx::AudioSourcePointer SoundSourceOggVorbis::open() const {
    return Mixxx::AudioSourceOggVorbis::create(getUrl());
}
