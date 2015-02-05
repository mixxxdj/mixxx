#include "sources/soundsourceflac.h"

#include "sources/audiosourceflac.h"

QList<QString> SoundSourceFLAC::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("flac");
    return list;
}

SoundSourceFLAC::SoundSourceFLAC(QUrl url)
        : SoundSource(url, "flac") {
}

Mixxx::AudioSourcePointer SoundSourceFLAC::open() const {
    return Mixxx::AudioSourceFLAC::create(getUrl());
}
