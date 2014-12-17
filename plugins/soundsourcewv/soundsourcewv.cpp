#include "soundsourcewv.h"

#include "audiosourcewv.h"
#include "trackmetadatataglib.h"

#include <taglib/wavpackfile.h>

namespace Mixxx {

QList<QString> SoundSourceWV::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("wv");
    return list;
}

SoundSourceWV::SoundSourceWV(QString fileName)
        : Super(fileName, "wv") {
}

Result SoundSourceWV::parseMetadata(Mixxx::TrackMetadata* pMetadata) const {
    TagLib::WavPack::File f(getFilename().toLocal8Bit().constData());

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
    TagLib::WavPack::File f(getFilename().toLocal8Bit().constData());
    TagLib::APE::Tag *ape = f.APETag();
    if (ape) {
        return Mixxx::readAPETagCover(*ape);
    } else {
        return QImage();
    }
}

Mixxx::AudioSourcePointer SoundSourceWV::open() const {
    return Mixxx::AudioSourceWV::open(getFilename());
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
    QList<QString> exts = Mixxx::SoundSourceWV::supportedFileExtensions();
    //Convert to C string array.
    char** c_exts = (char**)malloc((exts.count() + 1) * sizeof(char*));
    for (int i = 0; i < exts.count(); i++)
    {
        QByteArray qba = exts[i].toUtf8();
        c_exts[i] = strdup(qba.constData());
        qDebug() << c_exts[i];
    }
    c_exts[exts.count()] = NULL; //NULL terminate the list

    return c_exts;
}

extern "C" MY_EXPORT void freeFileExtensions(char **exts) {
    if (exts) {
        for (int i(0); exts[i]; ++i) {
            free(exts[i]);
        }
        free(exts);
    }
}
