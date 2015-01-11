#include "metadata/audiotagger.h"

#include "metadata/trackmetadatataglib.h"

#include "util/assert.h"

#include <taglib/vorbisfile.h>
#include <taglib/wavpackfile.h>
#include <taglib/mpegfile.h>
#include <taglib/mp4file.h>
#include <taglib/oggfile.h>
#include <taglib/flacfile.h>
#include <taglib/aifffile.h>
#include <taglib/rifffile.h>
#include <taglib/wavfile.h>
#if (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 9))
#include <taglib/opusfile.h>
#endif

#include <QtDebug>

AudioTagger::AudioTagger(const QString& file, SecurityTokenPointer pToken)
        : m_file(file),
          m_pSecurityToken(pToken.isNull() ? Sandbox::openSecurityToken(
                  m_file, true) : pToken) {
}

AudioTagger::~AudioTagger() {
}

bool AudioTagger::save(const Mixxx::TrackMetadata& trackMetadata) {
    QScopedPointer<TagLib::File> pFile;

    const QString filePath(m_file.canonicalFilePath());
    const QByteArray filePathByteArray(m_file.canonicalFilePath().toLocal8Bit());
    const char* filePathChars = filePathByteArray.constData();

    if (filePath.endsWith(".mp3", Qt::CaseInsensitive)) {
        QScopedPointer<TagLib::MPEG::File> pMpegFile(
                new TagLib::MPEG::File(filePathChars));
        writeID3v2Tag(pMpegFile->ID3v2Tag(true), trackMetadata); // mandatory
        writeAPETag(pMpegFile->APETag(false), trackMetadata); // optional
        pFile.reset(pMpegFile.take()); // transfer ownership
    } else if (filePath.endsWith(".m4a", Qt::CaseInsensitive)) {
        QScopedPointer<TagLib::MP4::File> pMp4File(
                new TagLib::MP4::File(filePathChars));
        writeMP4Tag(pMp4File->tag(), trackMetadata);
        pFile.reset(pMp4File.take()); // transfer ownership
    } else if (filePath.endsWith(".ogg", Qt::CaseInsensitive)) {
        QScopedPointer<TagLib::Ogg::Vorbis::File> pOggFile(
                new TagLib::Ogg::Vorbis::File(filePathChars));
        writeXiphComment(pOggFile->tag(), trackMetadata);
        pFile.reset(pOggFile.take()); // transfer ownership
    } else if (filePath.endsWith(".flac", Qt::CaseInsensitive)) {
        QScopedPointer<TagLib::FLAC::File> pFlacFile(
                new TagLib::FLAC::File(filePathChars));
        writeXiphComment(pFlacFile->xiphComment(true), trackMetadata); // mandatory
        writeID3v2Tag(pFlacFile->ID3v2Tag(false), trackMetadata); // optional
        pFile.reset(pFlacFile.take()); // transfer ownership
    } else if (filePath.endsWith(".wav", Qt::CaseInsensitive)) {
        QScopedPointer<TagLib::RIFF::WAV::File> pWavFile(
                new TagLib::RIFF::WAV::File(filePathChars));
        writeID3v2Tag(pWavFile->tag(), trackMetadata);
        pFile.reset(pWavFile.take()); // transfer ownership
    } else if (filePath.endsWith(".aif", Qt::CaseInsensitive) ||
            filePath.endsWith(".aiff", Qt::CaseInsensitive)) {
        QScopedPointer<TagLib::RIFF::AIFF::File> pAiffFile(
                new TagLib::RIFF::AIFF::File(filePathChars));
        writeID3v2Tag(pAiffFile->tag(), trackMetadata);
        pFile.reset(pAiffFile.take()); // transfer ownership
#if (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 9))
    } else if (filePath.endsWith(".opus", Qt::CaseInsensitive)) {
        QScopedPointer<TagLib::Ogg::Opus::File> pOpusFile(
                new TagLib::Ogg::Opus::File(filePathChars));
        writeXiphComment(pOpusFile->tag(), trackMetadata);
        pFile.reset(pOpusFile.take()); // transfer ownership
#endif
    } else {
        qWarning() << "Unsupported file type! Could not update metadata of track " << filePath;
        return false;
    }

    // write audio tags to file
    DEBUG_ASSERT(pFile);
    if (pFile->save()) {
        qDebug() << "Successfully updated metadata of track " << filePath;
        return true;
    } else {
        qWarning() << "Failed to update metadata of track " << filePath;
        return false;
    }
}
