#include "sources/soundsourcesndfile.h"

#include "sources/audiosourcesndfile.h"

QList<QString> SoundSourceSndFile::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("aiff");
    list.push_back("aif");
    list.push_back("wav");
    list.push_back("flac");
    return list;
}

SoundSourceSndFile::SoundSourceSndFile(QUrl url)
        : SoundSource(url) {
}

Mixxx::AudioSourcePointer SoundSourceSndFile::open() const {
    return Mixxx::AudioSourceSndFile::create(getUrl());
}
