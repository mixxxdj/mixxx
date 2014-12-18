/* -*- mode:C++; indent-tabs-mode:t; tab-width:8; c-basic-offset:4; -*- */
/***************************************************************************
                          soundsourceffmpeg.cpp -  ffmpeg decoder
                             -------------------
    copyright            : (C) 2007 by Cedric GESTES
                           (C) 2012-2015 by Tuukka Pasanen
    email                : tuukka.pasanen@ilmi.fi

    This one tested with FFMPEG 0.10/0.11/1.0/1.1/1.2/2,0/2,1/GIT
                         Libav  0.8/9/GIT
    FFMPEG below 0.10 WON'T work. If you like to it work you can
    allways send a patch but it's mostly not worth it!
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "sources/soundsourceffmpeg.h"

<<<<<<< HEAD:src/soundsourceffmpeg.cpp
<<<<<<< HEAD
<<<<<<< HEAD
#include "soundsourcetaglib.h"
#include <taglib/mpegfile.h>
#include <taglib/vorbisfile.h>
#include <taglib/mp4file.h>
#include <taglib/opusfile.h>
#include <taglib/flacfile.h>
#include <taglib/aifffile.h>
#include <taglib/wavfile.h>

#include <QtDebug>
=======
#include "trackmetadata.h"

>>>>>>> Move track metadata properties from SoundSource into separate DTO class
#include <QBuffer>
=======
#include "audiosourceffmpeg.h"
=======
#include "sources/audiosourceffmpeg.h"
<<<<<<< HEAD
>>>>>>> Move Audio-/SoundSources code into separate directory:src/sources/soundsourceffmpeg.cpp
#include "trackmetadata.h"
=======
#include "metadata/trackmetadata.h"
>>>>>>> Move metadata code into separate directory

>>>>>>> Split AudioSource from SoundSource
#include <QtDebug>

QList<QString> SoundSourceFFmpeg::supportedFileExtensions() {
    QList<QString> list;
    AVInputFormat *l_SInputFmt  = NULL;

    while ((l_SInputFmt = av_iformat_next(l_SInputFmt))) {
        if (l_SInputFmt->name == NULL) {
            break;
        }

        if (!strcmp(l_SInputFmt->name, "flac")) {
            list.append("flac");
        } else if (!strcmp(l_SInputFmt->name, "ogg")) {
            list.append("ogg");
        } else if (!strcmp(l_SInputFmt->name, "mov,mp4,m4a,3gp,3g2,mj2")) {
            list.append("m4a");
        } else if (!strcmp(l_SInputFmt->name, "mp4")) {
            list.append("mp4");
        } else if (!strcmp(l_SInputFmt->name, "mp3")) {
            list.append("mp3");
        } else if (!strcmp(l_SInputFmt->name, "aac")) {
            list.append("aac");
        } else if (!strcmp(l_SInputFmt->name, "opus") ||
                   !strcmp(l_SInputFmt->name, "libopus")) {
            list.append("opus");
        } else if (!strcmp(l_SInputFmt->name, "wma")) {
            list.append("wma");
        }
    }

    return list;
}

SoundSourceFFmpeg::SoundSourceFFmpeg(QString fileName)
    : Mixxx::SoundSource(fileName) {
}

<<<<<<< HEAD
<<<<<<< HEAD
Result SoundSourceFFmpeg::parseHeader() {
    QString location = getFilename();
    setType(location.section(".",-1).toLower());
=======
Result SoundSourceFFmpeg::parseMetadata(Mixxx::TrackMetadata* pMetadata) {
=======
Result SoundSourceFFmpeg::parseMetadata(Mixxx::TrackMetadata* pMetadata) const {
>>>>>>> Split AudioSource from SoundSource
    qDebug() << "ffmpeg: SoundSourceFFmpeg::parseMetadata" << getFilename();

    AVFormatContext *FmtCtx = avformat_alloc_context();
    AVCodecContext *CodecCtx = NULL;
    AVDictionaryEntry *FmtTag = NULL;
    unsigned int i;
    AVDictionary *l_iFormatOpts = NULL;
>>>>>>> Move track metadata properties from SoundSource into separate DTO class

<<<<<<< HEAD
    QByteArray qBAFilename = getFilename().toLocal8Bit();
=======
    if (avformat_open_input(&FmtCtx, getFilename().toLocal8Bit().constData(), NULL,
                            &l_iFormatOpts) !=0) {
        qDebug() << "av_open_input_file: cannot open" << getFilename();
        return ERR;
    }
>>>>>>> Eliminate unnecessary local variables

    bool is_flac = location.endsWith("flac", Qt::CaseInsensitive);
    bool is_wav = location.endsWith("wav", Qt::CaseInsensitive);
    bool is_ogg = location.endsWith("ogg", Qt::CaseInsensitive);
    bool is_mp3 = location.endsWith("mp3", Qt::CaseInsensitive);
    bool is_mp4 = location.endsWith("mp4", Qt::CaseInsensitive) || location.endsWith("m4a", Qt::CaseInsensitive);
    bool is_opus = location.endsWith("opus", Qt::CaseInsensitive);
    bool is_aiff = location.endsWith("aiff", Qt::CaseInsensitive);

    if (is_flac) {
        TagLib::FLAC::File f(qBAFilename.constData());
        if (!readFileHeader(this, f)) {
            return ERR;
        }
        TagLib::Ogg::XiphComment* xiph = f.xiphComment();
        if (xiph) {
            readXiphComment(this, *xiph);
        }
        else {
            TagLib::ID3v2::Tag *id3v2(f.ID3v2Tag());
            if (id3v2) {
                readID3v2Tag(this, *id3v2);
            } else {
                // fallback
                const TagLib::Tag *tag(f.tag());
                if (tag) {
                    readTag(this, *tag);
                } else {
                    return ERR;
                }
            }
        }
    } else if (is_wav) {
        TagLib::RIFF::WAV::File f(qBAFilename.constData());
        if (!readFileHeader(this, f)) {
            return ERR;
        }

        // Taglib provides the ID3v2Tag method for WAV files since Version 1.9
#if (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 9))
        TagLib::ID3v2::Tag* id3v2(f.ID3v2Tag());
        if (id3v2) {
            readID3v2Tag(this, *id3v2);
        } else {
            // fallback
            const TagLib::Tag* tag(f.tag());
            if (tag) {
                readTag(this, *tag);
            } else {
                return ERR;
            }
        }
#else
        TagLib::ID3v2::Tag* id3v2(f.tag());
        if (id3v2) {
            readID3v2Tag(this, *id3v2);
        }
#endif

<<<<<<< HEAD
    } else if (is_aiff) {
        // Try AIFF
        TagLib::RIFF::AIFF::File f(qBAFilename.constData());
        if (!readFileHeader(this, f)) {
            return ERR;
        }
        TagLib::ID3v2::Tag *id3v2(f.tag());
        if (id3v2) {
            readID3v2Tag(this, *id3v2);
        } else {
            return ERR;
        }
    } else if (is_mp3) {
        TagLib::MPEG::File f(qBAFilename.constData());
=======
    // Retrieve stream information
    if (avformat_find_stream_info(FmtCtx, NULL)<0) {
        qDebug() << "av_find_stream_info: Can't find metadata" <<
                getFilename();
        avcodec_close(CodecCtx);
        avformat_close_input(&FmtCtx);
        av_free(FmtCtx);
        return ERR;
    }
>>>>>>> Eliminate unnecessary local variables

<<<<<<< HEAD
        if (!readFileHeader(this, f)) {
            return ERR;
        }
<<<<<<< HEAD
=======
    if (m_iAudioStream==-1) {
=======
    int iAudioStream = -1;
    for (i = 0; i < FmtCtx->nb_streams; ++i) {
        if (FmtCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO) {
            iAudioStream = i;
            break;
        }
    }
    if (iAudioStream == -1) {
>>>>>>> Split AudioSource from SoundSource
        qDebug() << "cannot find an audio stream: Can't find stream" <<
                getFilename();
        avcodec_close(CodecCtx);
        avformat_close_input(&FmtCtx);
        av_free(FmtCtx);
        return ERR;
    }
>>>>>>> Eliminate unnecessary local variables

<<<<<<< HEAD
        // Now look for MP3 specific metadata (e.g. BPM)
        TagLib::ID3v2::Tag* id3v2 = f.ID3v2Tag();
        if (id3v2) {
            readID3v2Tag(this, *id3v2);
        } else {
            TagLib::APE::Tag *ape = f.APETag();
            if (ape) {
                readAPETag(this, *ape);
            } else {
                // fallback
                const TagLib::Tag *tag(f.tag());
                if (tag) {
                    readTag(this, *tag);
                } else {
                    return ERR;
                }
            }
=======
    // Get a pointer to the codec context for the video stream
    CodecCtx=FmtCtx->streams[iAudioStream]->codec;

    while ((FmtTag = av_dict_get(FmtCtx->metadata, "", FmtTag,
                                 AV_DICT_IGNORE_SUFFIX))) {
        QString strValue (QString::fromUtf8 (FmtTag->value));

        // TODO: More sophisticated metadata reading
        if (!strncmp(FmtTag->key, "artist", 7)) {
            pMetadata->setArtist(strValue);
        } else if (!strncmp(FmtTag->key, "title", 5)) {
            pMetadata->setTitle(strValue);
        } else if (!strncmp(FmtTag->key, "album_artist", 12)) {
            pMetadata->setAlbumArtist(strValue);
        } else if (!strncmp(FmtTag->key, "albumartist", 11)) {
            pMetadata->setAlbumArtist(strValue);
        } else if (!strncmp(FmtTag->key, "album", 5)) {
            pMetadata->setAlbum(strValue);
        } else if (!strncmp(FmtTag->key, "TOAL", 4)) {
            pMetadata->setAlbum(strValue);
        } else if (!strncmp(FmtTag->key, "date", 4)) {
            pMetadata->setYear(strValue);
        } else if (!strncmp(FmtTag->key, "genre", 5)) {
            pMetadata->setGenre(strValue);
        } else if (!strncmp(FmtTag->key, "comment", 7)) {
            pMetadata->setComment(strValue);
>>>>>>> Move track metadata properties from SoundSource into separate DTO class
        }
    } else if (is_ogg) {
        TagLib::Ogg::Vorbis::File f(qBAFilename.constData());

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
    } else if (is_mp4) {
        TagLib::MP4::File f(getFilename().toLocal8Bit().constData());

<<<<<<< HEAD
<<<<<<< HEAD
        if (!readFileHeader(this, f)) {
           return ERR;
=======
    while ((FmtTag = av_dict_get(FmtCtx->streams[m_iAudioStream]->metadata, "",
=======
    while ((FmtTag = av_dict_get(FmtCtx->streams[iAudioStream]->metadata, "",
>>>>>>> Split AudioSource from SoundSource
                                 FmtTag, AV_DICT_IGNORE_SUFFIX))) {
        // Convert the value from UTF-8.
        QString strValue (QString::fromUtf8 (FmtTag->value));

        if (!strncmp(FmtTag->key, "ARTIST", 7)) {
            pMetadata->setArtist(strValue);
        } else if (!strncmp(FmtTag->key, "ALBUM", 5)) {
            pMetadata->setAlbum(strValue);
        } else if (!strncmp(FmtTag->key, "YEAR", 4)) {
            pMetadata->setYear(strValue);
        } else if (!strncmp(FmtTag->key, "GENRE", 5)) {
            pMetadata->setGenre(strValue);
        } else if (!strncmp(FmtTag->key, "TITLE", 5)) {
            pMetadata->setTitle(strValue);
        } else if (!strncmp(FmtTag->key, "REPLAYGAIN_TRACK_PEAK", 20)) {
        } else if (!strncmp(FmtTag->key, "REPLAYGAIN_TRACK_GAIN", 20)) {
            pMetadata->setReplayGainString (strValue);
        } else if (!strncmp(FmtTag->key, "REPLAYGAIN_ALBUM_PEAK", 20)) {
        } else if (!strncmp(FmtTag->key, "REPLAYGAIN_ALBUM_GAIN", 20)) {
>>>>>>> Move track metadata properties from SoundSource into separate DTO class
        }

        TagLib::MP4::Tag *mp4(f.tag());

        if (mp4) {
          readMP4Tag(this, *mp4);
        } else {
          // fallback
          const TagLib::Tag *tag(f.tag());
          if (tag) {
            readTag(this, *tag);
          } else {
            return ERR;
          }
        }
    } else if (is_opus) {
        // If some have too old Taglib it's his own pain
        TagLib::Ogg::Opus::File f(qBAFilename.constData());

<<<<<<< HEAD
<<<<<<< HEAD
        if (!readFileHeader(this, f)) {
            return ERR;
        }
=======
    this->setChannels(CodecCtx->channels);
    this->setSampleRate(CodecCtx->sample_rate);
    this->setBitrate(CodecCtx->bit_rate / 1000);
    this->setDuration(FmtCtx->duration / AV_TIME_BASE);
>>>>>>> New SoundSource/AudioSource API
=======
    pMetadata->setChannels(CodecCtx->channels);
    pMetadata->setSampleRate(CodecCtx->sample_rate);
    pMetadata->setBitrate(CodecCtx->bit_rate / 1000);
    pMetadata->setDuration(FmtCtx->duration / AV_TIME_BASE);
>>>>>>> Move track metadata properties from SoundSource into separate DTO class

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
    }

    return OK;
}

<<<<<<< HEAD
QImage SoundSourceFFmpeg::parseCoverArt() {
<<<<<<< HEAD
    QString location = getFilename();
    setType(location.section(".",-1).toLower());

    QByteArray qBAFilename = getFilename().toLocal8Bit();
    QImage coverArt;

    if (getType() == "flac") {
        TagLib::FLAC::File f(qBAFilename.constData());
        TagLib::ID3v2::Tag* id3v2 = f.ID3v2Tag();
        if (id3v2) {
            coverArt = Mixxx::getCoverInID3v2Tag(*id3v2);
        }
        if (coverArt.isNull()) {
            TagLib::Ogg::XiphComment *xiph = f.xiphComment();
            if (xiph) {
                coverArt = Mixxx::getCoverInXiphComment(*xiph);
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
        TagLib::RIFF::WAV::File f(qBAFilename.constData());
        TagLib::ID3v2::Tag* id3v2 = f.tag();
        if (id3v2) {
            coverArt = Mixxx::getCoverInID3v2Tag(*id3v2);
        }
    } else if (getType() == "aiff") {
        // Try AIFF
        TagLib::RIFF::AIFF::File f(qBAFilename.constData());
        TagLib::ID3v2::Tag* id3v2 = f.tag();
        if (id3v2) {
            coverArt = Mixxx::getCoverInID3v2Tag(*id3v2);
        }
    } else if (getType() == "mp3") {
        TagLib::MPEG::File f(getFilename().toLocal8Bit().constData());
        TagLib::ID3v2::Tag* id3v2 = f.ID3v2Tag();
        if (id3v2) {
            coverArt = Mixxx::getCoverInID3v2Tag(*id3v2);
        }
        if (coverArt.isNull()) {
            TagLib::APE::Tag *ape = f.APETag();
            if (ape) {
                coverArt = Mixxx::getCoverInAPETag(*ape);
            }
        }
    } else if (getType() == "ogg" || getType() == "opus") {
        TagLib::Ogg::Vorbis::File f(getFilename().toLocal8Bit().constData());
        TagLib::Ogg::XiphComment *xiph = f.tag();
        if (xiph) {
            coverArt = Mixxx::getCoverInXiphComment(*xiph);
        }
   } else if (getType() == "mp4" || getType() == "m4a") {
        TagLib::MP4::File f(getFilename().toLocal8Bit().constData());
        TagLib::MP4::Tag *mp4(f.tag());
        if (mp4) {
            coverArt = Mixxx::getCoverInMP4Tag(*mp4);
        }
  }

  return coverArt;
=======
=======
QImage SoundSourceFFmpeg::parseCoverArt() const {
>>>>>>> Split AudioSource from SoundSource
    // currently not implemented
    return QImage();
>>>>>>> New SoundSource/AudioSource API
}

Mixxx::AudioSourcePointer SoundSourceFFmpeg::open() const {
    return Mixxx::AudioSourceFFmpeg::open(getFilename());
}
