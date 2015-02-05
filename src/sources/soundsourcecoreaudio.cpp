#include "sources/soundsourcecoreaudio.h"

#include "sources/audiosourcecoreaudio.h"

#include <QtDebug>

QList<QString> SoundSourceCoreAudio::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("m4a");
    list.push_back("mp3");
    list.push_back("mp2");
    //Can add mp3, mp2, ac3, and others here if you want.
    //See:
    //  http://developer.apple.com/library/mac/documentation/MusicAudio/Reference/AudioFileConvertRef/Reference/reference.html#//apple_ref/doc/c_ref/AudioFileTypeID

    //XXX: ... but make sure you implement handling for any new format in ParseHeader!!!!!! -- asantoni
    return list;
}

SoundSourceCoreAudio::SoundSourceCoreAudio(QUrl url)
        : SoundSource(url) {
}

Mixxx::AudioSourcePointer SoundSourceCoreAudio::open() const {
    return Mixxx::AudioSourceCoreAudio::create(getUrl());
}
