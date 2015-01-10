#include "sources/soundsourcesndfile.h"

#include "sources/audiosourcesndfile.h"
#include "metadata/trackmetadatataglib.h"

#include <taglib/flacfile.h>
#include <taglib/aifffile.h>
#include <taglib/rifffile.h>
#include <taglib/wavfile.h>

QList<QString> SoundSourceSndFile::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("aiff");
    list.push_back("aif");
    list.push_back("wav");
    list.push_back("flac");
    return list;
}

SoundSourceSndFile::SoundSourceSndFile(QString qFilename)
        : SoundSource(qFilename) {
}

Result SoundSourceSndFile::parseMetadata(Mixxx::TrackMetadata* pMetadata) const {
    if (getType() == "flac") {
        TagLib::FLAC::File f(getFilename().toLocal8Bit().constData());
        if (!readAudioProperties(pMetadata, f)) {
            return ERR;
        }
        TagLib::Ogg::XiphComment* xiph = f.xiphComment();
        if (xiph) {
            readXiphComment(pMetadata, *xiph);
        } else {
            TagLib::ID3v2::Tag *id3v2(f.ID3v2Tag());
            if (id3v2) {
                readID3v2Tag(pMetadata, *id3v2);
            } else {
                // fallback
                const TagLib::Tag *tag(f.tag());
                if (tag) {
                    readTag(pMetadata, *tag);
                } else {
                    return ERR;
                }
            }
        }
    } else if (getType() == "wav") {
        TagLib::RIFF::WAV::File f(getFilename().toLocal8Bit().constData());
        if (!readAudioProperties(pMetadata, f)) {
            return ERR;
        }
        TagLib::ID3v2::Tag *id3v2(f.ID3v2Tag());
        if (id3v2) {
            readID3v2Tag(pMetadata, *id3v2);
        } else {
            // fallback
            const TagLib::Tag *tag(f.tag());
            if (tag) {
                readTag(pMetadata, *tag);
            } else {
                return ERR;
            }
        }
    } else if (getType().startsWith("aif")) {
        // Try AIFF
        TagLib::RIFF::AIFF::File f(getFilename().toLocal8Bit().constData());
        if (!readAudioProperties(pMetadata, f)) {
            return ERR;
        }
        TagLib::ID3v2::Tag *id3v2(f.tag());
        if (id3v2) {
            readID3v2Tag(pMetadata, *id3v2);
        } else {
            return ERR;
        }
    } else {
        return ERR;
    }

    return OK;
}

QImage SoundSourceSndFile::parseCoverArt() const {
    QImage coverArt;

    if (getType() == "flac") {
        TagLib::FLAC::File f(getFilename().toLocal8Bit().constData());
        TagLib::ID3v2::Tag* id3v2 = f.ID3v2Tag();
        if (id3v2) {
            coverArt = Mixxx::readID3v2TagCover(*id3v2);
        }
        if (coverArt.isNull()) {
            TagLib::Ogg::XiphComment *xiph = f.xiphComment();
            if (xiph) {
                coverArt = Mixxx::readXiphCommentCover(*xiph);
            }
        }
        if (coverArt.isNull()) {
            TagLib::List<TagLib::FLAC::Picture*> covers = f.pictureList();
            if (!covers.isEmpty()) {
                std::list<TagLib::FLAC::Picture*>::iterator it = covers.begin();
                TagLib::FLAC::Picture* cover = *it;
                coverArt = QImage::fromData(
                        QByteArray(cover->data().data(), cover->data().size()));
            }
        }
    } else if (getType() == "wav") {
        TagLib::RIFF::WAV::File f(getFilename().toLocal8Bit().constData());
        TagLib::ID3v2::Tag* id3v2 = f.tag();
        if (id3v2) {
            coverArt = Mixxx::readID3v2TagCover(*id3v2);
        }
    } else if (getType().startsWith("aif")) {
        // Try AIFF
        TagLib::RIFF::AIFF::File f(getFilename().toLocal8Bit().constData());
        TagLib::ID3v2::Tag* id3v2 = f.tag();
        if (id3v2) {
            coverArt = Mixxx::readID3v2TagCover(*id3v2);
        }
    }

    return coverArt;
}

Mixxx::AudioSourcePointer SoundSourceSndFile::open() const {
    return Mixxx::AudioSourceSndFile::create(getFilename());
}
