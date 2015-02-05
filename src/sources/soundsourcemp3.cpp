#include "sources/soundsourcemp3.h"

#include "sources/audiosourcemp3.h"

QList<QString> SoundSourceMp3::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("mp3");
    return list;
}

SoundSourceMp3::SoundSourceMp3(QUrl url)
        : SoundSource(url, "mp3") {
}

Mixxx::AudioSourcePointer SoundSourceMp3::open() const {
    return Mixxx::AudioSourceMp3::create(getUrl());
}
