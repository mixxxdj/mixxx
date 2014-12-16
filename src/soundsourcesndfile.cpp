#include "soundsourcesndfile.h"

#include "trackmetadatataglib.h"

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
        : Super(qFilename), m_pSndFile(NULL) {
    memset(&m_sfInfo, 0, sizeof(m_sfInfo));
}

SoundSourceSndFile::~SoundSourceSndFile() {
    close();
}

Result SoundSourceSndFile::open() {
#ifdef __WINDOWS__
    // Pointer valid until string changed
    LPCWSTR lpcwFilename = (LPCWSTR)getFilename().utf16();
    m_pSndFile = sf_wchar_open(lpcwFilename, SFM_READ, &m_sfInfo);
#else
    m_pSndFile = sf_open(getFilename().toLocal8Bit().constData(), SFM_READ, &m_sfInfo);
#endif

    if (m_pSndFile == NULL) {   // sf_format_check is only for writes
        qWarning() << "libsndfile: Error opening file" << getFilename()
                << sf_strerror(m_pSndFile);
        return ERR;
    }

    if (sf_error(m_pSndFile) > 0) {
        qWarning() << "libsndfile: Error opening file" << getFilename()
                << sf_strerror(m_pSndFile);
        close();
        return ERR;
    }

    setChannelCount(m_sfInfo.channels);
    setFrameRate(m_sfInfo.samplerate);
    setFrameCount(m_sfInfo.frames);

    return OK;
}

void SoundSourceSndFile::close() {
    if (m_pSndFile) {
        if (0 == sf_close(m_pSndFile)) {
            m_pSndFile = NULL;
            memset(&m_sfInfo, 0, sizeof(m_sfInfo));
            Super::reset();
        } else {
            qWarning() << "Failed to close file:" << getFilename()
                    << sf_strerror(m_pSndFile);
        }
    }
}

Mixxx::AudioSource::diff_type SoundSourceSndFile::seekFrame(
        diff_type frameIndex) {
    const sf_count_t seekResult = sf_seek(m_pSndFile, frameIndex, SEEK_SET);
    if (0 <= seekResult) {
        return seekResult;
    } else {
        qWarning() << "Failed to seek libsnd file:" << getFilename()
                << sf_strerror(m_pSndFile);
        return sf_seek(m_pSndFile, 0, SEEK_CUR);
    }
}

Mixxx::AudioSource::size_type SoundSourceSndFile::readFrameSamplesInterleaved(
        size_type frameCount, sample_type* sampleBuffer) {
    sf_count_t readCount = sf_readf_float(m_pSndFile, sampleBuffer, frameCount);
    if (0 <= readCount) {
        return readCount;
    } else {
        qWarning() << "Failed to read sample data from libsnd file:"
                << getFilename() << sf_strerror(m_pSndFile);
        return 0;
    }
}

Result SoundSourceSndFile::parseMetadata(Mixxx::TrackMetadata* pMetadata) {

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

        if (pMetadata->getDuration() <= 0) {
            // we're using a taglib version which doesn't know how to do wav
            // durations, set it with m_sfInfo from sndfile -bkgood
            // XXX remove this when ubuntu ships with an sufficiently
            // intelligent version of taglib, should happen in 11.10

            // Have to open the file for m_sfInfo to be valid.
            if (m_pSndFile == NULL) {
                open();
            }

            if (m_sfInfo.samplerate > 0) {
                pMetadata->setDuration(m_sfInfo.frames / m_sfInfo.samplerate);
            } else {
                qDebug() << "WARNING: WAV file with invalid samplerate."
                        << "Can't get duration using libsndfile.";
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

QImage SoundSourceSndFile::parseCoverArt() {
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
