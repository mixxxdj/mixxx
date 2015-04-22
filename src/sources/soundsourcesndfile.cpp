#include "sources/soundsourcesndfile.h"

namespace Mixxx {

QList<QString> SoundSourceSndFile::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("aiff");
    list.push_back("aif");
    list.push_back("wav");
    list.push_back("flac");
    return list;
}

SoundSourceSndFile::SoundSourceSndFile(QUrl url)
        : SoundSource(url),
          m_pSndFile(NULL) {
}

<<<<<<< HEAD
<<<<<<< HEAD
Result SoundSourceSndFile::parseMetadata(
        Mixxx::TrackMetadata* pMetadata) const {
    if (getType() == "flac") {
        TagLib::FLAC::File f(getLocalFileNameBytes().constData());
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
        TagLib::RIFF::WAV::File f(getLocalFileNameBytes().constData());
        if (!readAudioProperties(pMetadata, f)) {
            return ERR;
        }
<<<<<<< HEAD

        // Taglib provides the ID3v2Tag method for WAV files since Version 1.9
#if (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 9))
=======
        // Taglib 1.8.x doesn't provide an ID3v2Tag method for WAV files.
#if TAGLIB_MAJOR_VERSION == 1 && TAGLIB_MINOR_VERSION == 8
        TagLib::ID3v2::Tag* id3v2(f.tag());
        if (id3v2) {
            readID3v2Tag(this, *id3v2);
        }
#else
>>>>>>> Add workaround for Taglib 1.8.x.
        TagLib::ID3v2::Tag* id3v2(f.ID3v2Tag());
        if (id3v2) {
            readID3v2Tag(pMetadata, *id3v2);
        } else {
            // fallback
            const TagLib::Tag* tag(f.tag());
            if (tag) {
                readTag(pMetadata, *tag);
            } else {
                return ERR;
            }
        }
<<<<<<< HEAD
<<<<<<< HEAD
#else
        TagLib::ID3v2::Tag* id3v2(f.tag());
        if (id3v2) {
            readID3v2Tag(this, *id3v2);
        }
#endif

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
=======
>>>>>>> Split AudioSource from SoundSource
=======
#endif
>>>>>>> Add workaround for Taglib 1.8.x.
    } else if (getType().startsWith("aif")) {
        // Try AIFF
        TagLib::RIFF::AIFF::File f(getLocalFileNameBytes().constData());
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
        TagLib::FLAC::File f(getLocalFileNameBytes().constData());
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
        TagLib::RIFF::WAV::File f(getLocalFileNameBytes().constData());
        TagLib::ID3v2::Tag* id3v2 = f.tag();
        if (id3v2) {
            coverArt = Mixxx::readID3v2TagCover(*id3v2);
        }
    } else if (getType().startsWith("aif")) {
        // Try AIFF
        TagLib::RIFF::AIFF::File f(getLocalFileNameBytes().constData());
        TagLib::ID3v2::Tag* id3v2 = f.tag();
        if (id3v2) {
            coverArt = Mixxx::readID3v2TagCover(*id3v2);
        }
    }

    return coverArt;
}

=======
>>>>>>> Universal tag reading function for all TagLib file types
Mixxx::AudioSourcePointer SoundSourceSndFile::open() const {
    return Mixxx::AudioSourceSndFile::create(getUrl());
=======
SoundSourceSndFile::~SoundSourceSndFile() {
    close();
>>>>>>> Move code from specialized AudioSources back into corresponding SoundSources
}

Result SoundSourceSndFile::tryOpen(const AudioSourceConfig& /*audioSrcCfg*/) {
    DEBUG_ASSERT(!m_pSndFile);
    memset(&m_sfInfo, 0, sizeof(m_sfInfo));
#ifdef __WINDOWS__
    // Pointer valid until string changed
    const QString fileName(getLocalFileName());
    LPCWSTR lpcwFilename = (LPCWSTR) fileName.utf16();
    m_pSndFile = sf_wchar_open(lpcwFilename, SFM_READ, &m_sfInfo);
#else
    m_pSndFile = sf_open(getLocalFileNameBytes().constData(), SFM_READ,
            &m_sfInfo);
#endif

    if (!m_pSndFile) {   // sf_format_check is only for writes
        qWarning() << "Error opening libsndfile file:" << getUrlString()
                << sf_strerror(m_pSndFile);
        return ERR;
    }

    if (sf_error(m_pSndFile) > 0) {
        qWarning() << "Error opening libsndfile file:" << getUrlString()
                << sf_strerror(m_pSndFile);
        return ERR;
    }

    setChannelCount(m_sfInfo.channels);
    setFrameRate(m_sfInfo.samplerate);
    setFrameCount(m_sfInfo.frames);

    return OK;
}

void SoundSourceSndFile::close() {
    if (m_pSndFile) {
        const int closeResult = sf_close(m_pSndFile);
        if (0 == closeResult) {
            m_pSndFile = NULL;
        } else {
            qWarning() << "Failed to close file:" << closeResult
                    << sf_strerror(m_pSndFile)
                    << getUrlString();
        }
    }
}

SINT SoundSourceSndFile::seekSampleFrame(
        SINT frameIndex) {
    DEBUG_ASSERT(isValidFrameIndex(frameIndex));

    const sf_count_t seekResult = sf_seek(m_pSndFile, frameIndex, SEEK_SET);
    if (0 <= seekResult) {
        return seekResult;
    } else {
        qWarning() << "Failed to seek libsnd file:" << seekResult
                << sf_strerror(m_pSndFile);
        return sf_seek(m_pSndFile, 0, SEEK_CUR);
    }
}

SINT SoundSourceSndFile::readSampleFrames(
        SINT numberOfFrames, CSAMPLE* sampleBuffer) {
    const sf_count_t readCount =
            sf_readf_float(m_pSndFile, sampleBuffer, numberOfFrames);
    if (0 <= readCount) {
        return readCount;
    } else {
        qWarning() << "Failed to read from libsnd file:" << readCount
                << sf_strerror(m_pSndFile);
        return 0;
    }
}

} // namespace Mixxx
