// soundsourceopus.cpp  -  Opus decoder
// Create by 14/01/2013 Tuukka Pasanen
// Based on work 2003 by Svein Magne Bang


#include "soundsourceopus.h"
#include "soundsourcetaglib.h"

#include <QtDebug>
#ifdef __WINDOWS__
#include <io.h>
#include <fcntl.h>
#elif __APPLE__
#include <CoreFoundation/CFByteOrder.h>
#elif __LINUX__
#include <endian.h>
#endif

// Include this if taglib if new enough (version 1.9.1 have opusfile)
#if (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 9))
#include <taglib/opusfile.h>
#endif

inline int getByteOrder() {
#ifdef __LINUX__
    return __BYTE_ORDER == __LITTLE_ENDIAN ? 0 : 1;
#elif __APPLE__
    return CFByteOrderGetCurrent() == CFByteOrderLittleEndian ? 0 : 1;
#else
    // Assume little endian.
    // TODO(XXX) BSD.
    return 0;
#endif

}

//
//   Class for reading Xiph Opus
//

SoundSourceOpus::SoundSourceOpus(QString qFilename)
    : Mixxx::SoundSource(qFilename),
    m_ptrOpusFile(NULL),
    m_lFilelength(0) {
    this->setType("opus");
}

SoundSourceOpus::~SoundSourceOpus() {
    if (m_ptrOpusFile) {
        op_free(m_ptrOpusFile);
    }
}

Result SoundSourceOpus::open() {
    int error = 0;
    const QByteArray qBAFilename(getFilename().toLocal8Bit());

    m_ptrOpusFile = op_open_file(qBAFilename.constData(), &error);
    if (m_ptrOpusFile == NULL) {
        qDebug() << "opus: Input does not appear to be an Opus bitstream.";
        m_lFilelength = 0;
        return ERR;
    }

    // opusfile lib all ways gives you 48000 samplerate and stereo 16 bit sample
    this->setBitrate(op_bitrate_instant(m_ptrOpusFile));
    this->setSampleRate(48000);
    this->setChannels(2);

    if (getChannels() > 2) {
        qDebug() << "opus: No support for more than 2 channels!";
        op_free(m_ptrOpusFile);
        m_lFilelength = 0;
        return ERR;
    }

    // op_pcm_total returns the total number of frames in the ogg file. The
    // frame is the channel-independent measure of samples. The total samples in
    // the file is m_iChannels * ov_pcm_total. rryan 7/2009 I verified this by
    // hand. a 30 second long 48khz mono ogg and a 48khz stereo ogg both report
    // 1440000 for op_pcm_total.
    qint64 ret = op_pcm_total(m_ptrOpusFile, -1) * 2;

    // qDebug() << getFilename() << "chan:" << m_iChannels << "sample:" << m_iSampleRate << "LEN:" << ret;


    if (ret >= 0) {
        // We pretend that the file is stereo to the rest of the world.
        m_lFilelength = ret;
    } else { //error
        if (ret == OP_EINVAL) {
            //The file is not seekable. Not sure if any action is needed.
            qDebug() << "opus: file is not seekable " << getFilename();
        }
    }

    return OK;
}

/*
   seek to <filepos>
 */

long SoundSourceOpus::seek(long filepos) {
    // In our speak, filepos is a sample in the file abstraction (i.e. it's
    // stereo no matter what). filepos/2 is the frame we want to seek to.
    if (filepos % 2 != 0) {
        qDebug() << "SoundSourceOpus got non-even seek target.";
        filepos--;
    }

    if (op_seekable(m_ptrOpusFile)) {
        // I can't say why filepos have to divide by two
        // Have no idea.. probably seek point is mono..
        if (op_pcm_seek(m_ptrOpusFile, filepos / 2) != 0) {
            // This is totally common (i.e. you're at EOF). Let's not leave this
            // qDebug on.

            qDebug() << "opus: Seek ERR on seekable.";
        }

        // qDebug() << "Wanted:" << filepos << "GOT:" << op_pcm_tell(m_ptrOpusFile);


        //return op_pcm_tell(m_ptrOpusFile);
        // We are here allways!
        return filepos;
    } else {
        qDebug() << "opus: Seek ERR at file " << getFilename();
        return 0;
    }
    return filepos;
}


/*
   read <size> samples into <destination>, and return the number of
   samples actually read.
 */

unsigned SoundSourceOpus::read(volatile unsigned long size, const SAMPLE * destination) {
    if (size % 2 != 0) {
        qDebug() << "SoundSourceOpus got non-even size in read.";
        size--;
    }

    // SAMPLE and opus_int16 are mostly same
    // So just make pointer and hope for the best
    opus_int16 *l_iDest = (opus_int16 *) destination;

    unsigned int l_iNeeded = size;
    unsigned int l_iReaded = 0;
    unsigned int l_iRet=0;

    // loop until requested number of samples has been retrieved
    while (l_iNeeded > 0) {
        // read samples into buffer
        //ret = op_read_stereo(m_ptrOpusFile, l_iPcm, sizeof(l_iPcm)/sizeof(*l_iPcm));
        l_iRet = op_read_stereo(m_ptrOpusFile, l_iDest, l_iNeeded);

        if (l_iRet <= 0) {
            // An error or EOF occured, break out and return what we have sofar.
            break;
        }

        l_iNeeded -= l_iRet * 2;
        l_iReaded += l_iRet * 2;
        l_iDest += l_iRet * 2;
    }

    return l_iReaded;
}

/*
   Parse the the file to get metadata
 */
Result SoundSourceOpus::parseHeader() {
    int error = 0;

    QByteArray qBAFilename = getFilename().toLocal8Bit();

    OggOpusFile *l_ptrOpusFile = op_open_file(qBAFilename.constData(), &error);
    this->setBitrate((int)op_bitrate(l_ptrOpusFile, -1) / 1000);
    this->setSampleRate(48000);
    this->setChannels(2);
    qint64 l_lLength = op_pcm_total(l_ptrOpusFile, -1) * getChannels();
    this->setDuration(l_lLength / (getSampleRate() * getChannels()));

// If we don't have new enough Taglib we use libopusfile parser!
#if (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 9))
    TagLib::Ogg::Opus::File f(qBAFilename.constData());

    if (!readFileHeader(this, f)) {
        return ERR;
    }

    TagLib::Ogg::XiphComment *xiph = f.tag();
    if (xiph) {
        readXiphComment(this, *xiph);
    } else {
        // fallback
        const TagLib::Tag *tag(f.tag());
        if (tag) {
            readTag(this, *tag);
        } else {
            return ERR;
        }
    }
#else
    // From Taglib 1.9.x Opus is supported
    // Before that we have parse tags by this code
    int i = 0;
    const OpusTags *l_ptrOpusTags = op_tags(l_ptrOpusFile, -1);


    // This is left for debug reasons !!
    // qDebug() << "opus: We have " << l_ptrOpusTags->comments;
    for (i = 0; i < l_ptrOpusTags->comments; ++i) {
      QString l_SWholeTag = QString(l_ptrOpusTags->user_comments[i]);
      QString l_STag = l_SWholeTag.left(l_SWholeTag.indexOf("="));
      QString l_SPayload = l_SWholeTag.right((l_SWholeTag.length() - l_SWholeTag.indexOf("=")) - 1);

      if (!l_STag.compare("ARTIST")) {
            this->setArtist(l_SPayload);
      } else if (!l_STag.compare("ALBUM")) {
            this->setAlbum(l_SPayload);
      } else if (!l_STag.compare("BPM")) {
            this->setBpm(l_SPayload.toFloat());
      } else if (!l_STag.compare("YEAR") || !l_STag.compare("DATE")) {
            this->setYear(l_SPayload);
      } else if (!l_STag.compare("GENRE")) {
            this->setGenre(l_SPayload);
      } else if (!l_STag.compare("TRACKNUMBER")) {
            this->setTrackNumber(l_SPayload);
      } else if (!l_STag.compare("COMPOSER")) {
            this->setComposer(l_SPayload);
      } else if (!l_STag.compare("ALBUMARTIST")) {
            this->setAlbumArtist(l_SPayload);
      } else if (!l_STag.compare("TITLE")) {
            this->setTitle(l_SPayload);
      } else if (!l_STag.compare("REPLAYGAIN_TRACK_PEAK")) {
      } else if (!l_STag.compare("REPLAYGAIN_TRACK_GAIN")) {
            this->setReplayGainString (l_SPayload);
      } else if (!l_STag.compare("REPLAYGAIN_ALBUM_PEAK")) {
      } else if (!l_STag.compare("REPLAYGAIN_ALBUM_GAIN")) {
      }

      // This is left fot debug reasons!!
      //qDebug() << "Comment" << i << l_ptrOpusTags->comment_lengths[i] <<
      //" (" << l_ptrOpusTags->user_comments[i] << ")" << l_STag << "*" << l_SPayload;
    }

    op_free(l_ptrOpusFile);
#endif

    return OK;
}

/*
   Return the length of the file in samples.
 */

inline long unsigned SoundSourceOpus::length() {
    return m_lFilelength;
}

QList<QString> SoundSourceOpus::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("opus");
    return list;
}

QImage SoundSourceOpus::parseCoverArt() {
#if (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 9))
    TagLib::Ogg::Opus::File f(getFilename().toLocal8Bit().constData());
    TagLib::Ogg::XiphComment *xiph = f.tag();
    if (xiph) {
        return Mixxx::getCoverInXiphComment(*xiph);
    }
#endif
    return QImage();
}
