/* -*- mode:C++; indent-tabs-mode:t; tab-width:8; c-basic-offset:4; -*- */
/***************************************************************************
                          soundsourceffmpeg.cpp -  ffmpeg decoder
                             -------------------
    copyright            : (C) 2007 by Cedric GESTES
                           (C) 2012-2014 by Tuukka Pasanen
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

#include "soundsourceffmpeg.h"

#include "audiosourceffmpeg.h"
#include "trackmetadata.h"

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

Result SoundSourceFFmpeg::parseMetadata(Mixxx::TrackMetadata* pMetadata) const {
    qDebug() << "ffmpeg: SoundSourceFFmpeg::parseMetadata" << getFilename();

    AVFormatContext *FmtCtx = avformat_alloc_context();
    AVCodecContext *CodecCtx = NULL;
    AVDictionaryEntry *FmtTag = NULL;
    unsigned int i;
    AVDictionary *l_iFormatOpts = NULL;

    if (avformat_open_input(&FmtCtx, getFilename().toLocal8Bit().constData(), NULL,
                            &l_iFormatOpts) !=0) {
        qDebug() << "av_open_input_file: cannot open" << getFilename();
        return ERR;
    }

#ifndef CODEC_ID_MP3
    if (LIBAVFORMAT_VERSION_INT > 3540580 && l_iFormatOpts != NULL) {
        av_dict_free(&l_iFormatOpts);
    }
#endif

#if LIBAVCODEC_VERSION_INT < 3622144
    FmtCtx->max_analyze_duration = 999999999;
#endif

    // Retrieve stream information
    if (avformat_find_stream_info(FmtCtx, NULL)<0) {
        qDebug() << "av_find_stream_info: Can't find metadata" <<
                getFilename();
        avcodec_close(CodecCtx);
        avformat_close_input(&FmtCtx);
        av_free(FmtCtx);
        return ERR;
    }

    int iAudioStream = -1;
    for (i = 0; i < FmtCtx->nb_streams; ++i) {
        if (FmtCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO) {
            iAudioStream = i;
            break;
        }
    }
    if (iAudioStream == -1) {
        qDebug() << "cannot find an audio stream: Can't find stream" <<
                getFilename();
        avcodec_close(CodecCtx);
        avformat_close_input(&FmtCtx);
        av_free(FmtCtx);
        return ERR;
    }

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
        }


    }

    while ((FmtTag = av_dict_get(FmtCtx->streams[iAudioStream]->metadata, "",
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
        }


    }

    pMetadata->setChannels(CodecCtx->channels);
    pMetadata->setSampleRate(CodecCtx->sample_rate);
    pMetadata->setBitrate(CodecCtx->bit_rate / 1000);
    pMetadata->setDuration(FmtCtx->duration / AV_TIME_BASE);

    avcodec_close(CodecCtx);
    avformat_close_input(&FmtCtx);
    av_free(FmtCtx);

    return OK;
}

QImage SoundSourceFFmpeg::parseCoverArt() const {
    // currently not implemented
    return QImage();
}

Mixxx::AudioSourcePointer SoundSourceFFmpeg::open() const {
    return Mixxx::AudioSourceFFmpeg::open(getFilename());
}
