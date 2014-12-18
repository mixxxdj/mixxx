#include "audiotagger.h"

#include "metadata/trackmetadatataglib.h"

#include <taglib/vorbisfile.h>
#include <taglib/wavpackfile.h>
#include <taglib/mpegfile.h>
#include <taglib/mp4file.h>
#include <taglib/oggfile.h>
#include <taglib/flacfile.h>
#include <taglib/aifffile.h>
#include <taglib/rifffile.h>
#include <taglib/wavfile.h>

#include <QtDebug>

AudioTagger::AudioTagger(const QString& file, SecurityTokenPointer pToken)
        : m_file(file),
          m_pSecurityToken(pToken.isNull() ? Sandbox::openSecurityToken(
                  m_file, true) : pToken) {
}

AudioTagger::~AudioTagger() {
}

bool AudioTagger::save(const Mixxx::TrackMetadata& trackMetadata) {
    TagLib::File* file = NULL;

    const QString& filePath = m_file.canonicalFilePath();
    QByteArray fileBA = filePath.toLocal8Bit();

    if (filePath.endsWith(".mp3", Qt::CaseInsensitive)) {
        file = new TagLib::MPEG::File(fileBA.constData());
        // process special ID3 fields, APEv2 fiels, etc

        // If the mp3 has no ID3v2 tag, we create a new one and add the TBPM and TKEY frame
        writeID3v2Tag(((TagLib::MPEG::File*) file)->ID3v2Tag(true), trackMetadata);
        // If the mp3 has an APE tag, we update
        writeAPETag(((TagLib::MPEG::File*) file)->APETag(false), trackMetadata);
    }

    if (filePath.endsWith(".m4a", Qt::CaseInsensitive)) {
        file = new TagLib::MP4::File(fileBA.constData());
        // process special ID3 fields, APEv2 fiels, etc
        writeMP4Tag(((TagLib::MP4::File*) file)->tag(), trackMetadata);
    }
    if (filePath.endsWith(".ogg", Qt::CaseInsensitive)) {
        file = new TagLib::Ogg::Vorbis::File(fileBA.constData());
        // process special ID3 fields, APEv2 fiels, etc
        writeXiphComment(((TagLib::Ogg::Vorbis::File*)file)->tag(), trackMetadata);
    }
    if (filePath.endsWith(".wav", Qt::CaseInsensitive)) {
        file = new TagLib::RIFF::WAV::File(fileBA.constData());
        //If the flac has no ID3v2 tag, we create a new one and add the TBPM and TKEY frame
        writeID3v2Tag(((TagLib::RIFF::WAV::File*)file)->tag(), trackMetadata);
    }
    if (filePath.endsWith(".flac", Qt::CaseInsensitive)) {
        file = new TagLib::FLAC::File(fileBA.constData());

        //If the flac has no ID3v2 tag, we create a new one and add the TBPM and TKEY frame
        writeID3v2Tag(((TagLib::FLAC::File*)file)->ID3v2Tag(true), trackMetadata);
        //If the flac has no APE tag, we create a new one and add the TBPM and TKEY frame
        writeXiphComment(((TagLib::FLAC::File*) file)->xiphComment(true), trackMetadata);

    }
    if (filePath.endsWith(".aif", Qt::CaseInsensitive) ||
            filePath.endsWith(".aiff", Qt::CaseInsensitive)) {
        file = new TagLib::RIFF::AIFF::File(fileBA.constData());
        //If the flac has no ID3v2 tag, we create a new one and add the TBPM and TKEY frame
        writeID3v2Tag(((TagLib::RIFF::AIFF::File*)file)->tag(), trackMetadata);

    }

    //process standard tags
    if (file) {
        TagLib::Tag *tag = file->tag();
        if (tag) {
            writeTag(tag, trackMetadata);
        }
        //write audio tags to file
        int success = file->save();
        if (success) {
            qDebug() << "Successfully updated metadata of track " << filePath;
        } else {
            qDebug() << "Could not update metadata of track " << filePath;
        }
        //delete file and return
        delete file;
        return success;
    } else {
        return false;
    }
}
