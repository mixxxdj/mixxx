#include "soundsourcewv.h"
#include "audiosourcewv.h"

#include "metadata/trackmetadatataglib.h"

#include <taglib/wavpackfile.h>

namespace Mixxx {

QList<QString> SoundSourceWV::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("wv");
    return list;
}

SoundSourceWV::SoundSourceWV(QUrl url)
        : SoundSourcePlugin(url, "wv") {
}

Result SoundSourceWV::parseMetadata(Mixxx::TrackMetadata* pMetadata) const {
    TagLib::WavPack::File f(getLocalFileNameBytes().constData());

    if (!readAudioProperties(pMetadata, f)) {
        return ERR;
    }

    TagLib::APE::Tag *ape = f.APETag();
    if (ape) {
        readAPETag(pMetadata, *ape);
    } else {
        // fallback
        const TagLib::Tag *tag(f.tag());
        if (tag) {
            readTag(pMetadata, *tag);
        } else {
            return ERR;
        }
    }

    return OK;
}

QImage SoundSourceWV::parseCoverArt() const {
    TagLib::WavPack::File f(getLocalFileNameBytes().constData());
    TagLib::APE::Tag *ape = f.APETag();
    if (ape) {
        return Mixxx::readAPETagCover(*ape);
    } else {
        return QImage();
    }
}

Mixxx::AudioSourcePointer SoundSourceWV::open() const {
    return Mixxx::AudioSourceWV::create(getUrl());
}

}  // namespace Mixxx

extern "C" MY_EXPORT const char* getMixxxVersion() {
    return VERSION;
}

extern "C" MY_EXPORT int getSoundSourceAPIVersion() {
    return MIXXX_SOUNDSOURCE_API_VERSION;
}

extern "C" MY_EXPORT Mixxx::SoundSource* getSoundSource(QString fileName) {
    return new Mixxx::SoundSourceWV(fileName);
}

extern "C" MY_EXPORT char** supportedFileExtensions() {
    const QList<QString> supportedFileExtensions(
            Mixxx::SoundSourceWV::supportedFileExtensions());
    return Mixxx::SoundSourcePlugin::allocFileExtensions(
            supportedFileExtensions);
}

extern "C" MY_EXPORT void freeFileExtensions(char** fileExtensions) {
    Mixxx::SoundSourcePlugin::freeFileExtensions(fileExtensions);
}
