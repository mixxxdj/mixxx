<<<<<<< HEAD
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

=======
>>>>>>> Review: Delete old file headers
#include "sources/soundsourceffmpeg.h"

<<<<<<< HEAD
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
=======
#include <vector>

#define AUDIOSOURCEFFMPEG_CACHESIZE 1000
#define AUDIOSOURCEFFMPEG_POSDISTANCE ((1024 * 1000) / 8)

namespace Mixxx {
>>>>>>> Move code from specialized AudioSources back into corresponding SoundSources

QStringList SoundSourceFFmpeg::supportedFileExtensions() {
    QStringList list;
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

SoundSourceFFmpeg::SoundSourceFFmpeg(QUrl url)
    : SoundSource(url),
      m_pFormatCtx(NULL),
      m_iAudioStream(-1),
      m_pCodecCtx(NULL),
      m_pCodec(NULL),
      m_pResample(NULL),
      m_currentMixxxFrameIndex(0),
      m_bIsSeeked(false),
      m_lCacheFramePos(0),
      m_lCacheStartFrame(0),
      m_lCacheEndFrame(0),
      m_lCacheLastPos(0),
      m_lLastStoredPos(0),
      m_lStoredSeekPoint(-1) {
}

SoundSourceFFmpeg::~SoundSourceFFmpeg() {
    close();
}

Result SoundSourceFFmpeg::tryOpen(const AudioSourceConfig& /*audioSrcCfg*/) {
    unsigned int i;
    AVDictionary *l_iFormatOpts = NULL;

    const QByteArray qBAFilename(getLocalFileNameBytes());
    qDebug() << "New SoundSourceFFmpeg :" << qBAFilename;

    DEBUG_ASSERT(!m_pFormatCtx);
    m_pFormatCtx = avformat_alloc_context();

#if LIBAVCODEC_VERSION_INT < 3622144
    m_pFormatCtx->max_analyze_duration = 999999999;
#endif

    // Open file and make m_pFormatCtx
    if (avformat_open_input(&m_pFormatCtx, qBAFilename.constData(), NULL,
                            &l_iFormatOpts)!=0) {
        qDebug() << "av_open_input_file: cannot open" << qBAFilename;
        return ERR;
    }

#if LIBAVCODEC_VERSION_INT > 3544932
    av_dict_free(&l_iFormatOpts);
#endif

    // Retrieve stream information
    if (avformat_find_stream_info(m_pFormatCtx, NULL)<0) {
        qDebug() << "av_find_stream_info: cannot open" << qBAFilename;
        return ERR;
    }

    //debug only (Enable if needed)
    //av_dump_format(m_pFormatCtx, 0, qBAFilename.constData(), false);

    // Find the first audio stream
    m_iAudioStream=-1;

    for (i=0; i<m_pFormatCtx->nb_streams; i++)
        if (m_pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO) {
            m_iAudioStream=i;
            break;
        }
    if (m_iAudioStream == -1) {
        qDebug() << "ffmpeg: cannot find an audio stream: cannot open"
                 << qBAFilename;
        return ERR;
    }

    // Get a pointer to the codec context for the audio stream
    m_pCodecCtx=m_pFormatCtx->streams[m_iAudioStream]->codec;

    // Find the decoder for the audio stream
    if (!(m_pCodec=avcodec_find_decoder(m_pCodecCtx->codec_id))) {
        qDebug() << "ffmpeg: cannot find a decoder for" << qBAFilename;
        return ERR;
    }

    if (avcodec_open2(m_pCodecCtx, m_pCodec, NULL)<0) {
        qDebug() << "ffmpeg:  cannot open" << qBAFilename;
        return ERR;
    }

    m_pResample = new EncoderFfmpegResample(m_pCodecCtx);
    m_pResample->open(m_pCodecCtx->sample_fmt, AV_SAMPLE_FMT_FLT);

    setChannelCount(m_pCodecCtx->channels);
    setFrameRate(m_pCodecCtx->sample_rate);
    setFrameCount((m_pFormatCtx->duration * m_pCodecCtx->sample_rate) / AV_TIME_BASE);

    qDebug() << "ffmpeg: Samplerate: " << getFrameRate() << ", Channels: " <<
             getChannelCount() << "\n";
    if (getChannelCount() > 2) {
        qDebug() << "ffmpeg: No support for more than 2 channels!";
        return ERR;
    }

    return OK;
}

void SoundSourceFFmpeg::close() {
    clearCache();

    if (m_pCodecCtx != NULL) {
        qDebug() << "~SoundSourceFFmpeg(): Clear FFMPEG stuff";
        avcodec_close(m_pCodecCtx);
        avformat_close_input(&m_pFormatCtx);
        av_free(m_pFormatCtx);
        m_pFormatCtx = NULL;
    }

    if (m_pResample != NULL) {
        qDebug() << "~SoundSourceFFmpeg(): Delete FFMPEG Resampler";
        delete m_pResample;
        m_pResample = NULL;
    }

    while (m_SJumpPoints.size() > 0) {
        ffmpegLocationObject* l_SRmJmp = m_SJumpPoints[0];
        m_SJumpPoints.remove(0);
        free(l_SRmJmp);
    }
}

void SoundSourceFFmpeg::clearCache() {
    while (m_SCache.size() > 0) {
        struct ffmpegCacheObject* l_SRmObj = m_SCache[0];
        m_SCache.remove(0);
        free(l_SRmObj->bytes);
        free(l_SRmObj);
    }
}

bool SoundSourceFFmpeg::readFramesToCache(unsigned int count, SINT offset) {
    unsigned int l_iCount = 0;
    qint32 l_iRet = 0;
    AVPacket l_SPacket;
    AVFrame *l_pFrame = NULL;
    bool l_iStop = false;
    int l_iFrameFinished = 0;
    struct ffmpegCacheObject *l_SObj = NULL;
    struct ffmpegCacheObject *l_SRmObj = NULL;
    bool m_bUnique = false;
    qint64 l_lLastPacketPos = -1;
    int l_iError = 0;
    int l_iFrameCount = 0;

    l_iCount = count;

    l_SPacket.data = NULL;
    l_SPacket.size = 0;


    while (l_iCount > 0) {
        if (l_pFrame != NULL) {
          l_iFrameCount --;
// FFMPEG 2.2 3561060 anb beyond
#if LIBAVCODEC_VERSION_INT >= 3561060
            av_frame_free(&l_pFrame);
// FFMPEG 0.11 and below
#elif LIBAVCODEC_VERSION_INT <= 3544932
            av_free(l_pFrame);
// FFMPEG 1.0 - 2.1
#else
            avcodec_free_frame(&l_pFrame);
#endif
            l_pFrame = NULL;

        }

        if (l_iStop == true) {
            break;
        }
        l_iFrameCount ++;
        av_init_packet(&l_SPacket);
#if LIBAVCODEC_VERSION_INT < 3617792
        l_pFrame = avcodec_alloc_frame();
#else
        l_pFrame = av_frame_alloc();
#endif

        // Read one frame (which has nothing to do with Mixxx Frame)
        // it's some packed audio data from container like MP3, Ogg or MP4
        if (av_read_frame(m_pFormatCtx, &l_SPacket) >= 0) {
            // Are we on correct audio stream. Currently we are always
            // Using first audio stream but in future there should be
            // possibility to choose which to use
            if (l_SPacket.stream_index == m_iAudioStream) {
                if (m_lStoredSeekPoint > 0) {
                    // Seek for correct jump point
                    if (m_lStoredSeekPoint > l_SPacket.pos &&
                            m_lStoredSeekPoint >= AUDIOSOURCEFFMPEG_POSDISTANCE) {
                        av_free_packet(&l_SPacket);
                        l_SPacket.data = NULL;
                        l_SPacket.size = 0;
                        continue;
                    }
                    m_lStoredSeekPoint = -1;
                }

                // Decode audio bytes (These can be S16P or FloatP [P is Planar])
                l_iRet = avcodec_decode_audio4(m_pCodecCtx,l_pFrame,&l_iFrameFinished,
                                               &l_SPacket);

                if (l_iRet <= 0) {
                    // An error or EOF occured,index break out and return what
                    // we have so far.
                    qDebug() << "EOF!";
                    l_iStop = true;
                    continue;
                } else {
                    l_iRet = 0;
                    l_SObj = (struct ffmpegCacheObject *)malloc(sizeof(struct ffmpegCacheObject));
                    if (l_SObj == NULL) {
                        qDebug() << "SoundSourceFFmpeg::readFramesToCache: Not enough memory!";
                        l_iStop = true;
                        continue;
                    }
                    memset(l_SObj, 0x00, sizeof(struct ffmpegCacheObject));

                    // Try to convert it to Mixxx understand output format
                    // which is pure Stereo Float
                    l_iRet = m_pResample->reSample(l_pFrame, &l_SObj->bytes);

                    if (l_iRet > 0) {
                        // Remove from cache
                        if (m_SCache.size() >= (AUDIOSOURCEFFMPEG_CACHESIZE - 10)) {
                            l_SRmObj = m_SCache[0];
                            m_SCache.remove(0);
                            free(l_SRmObj->bytes);
                            free(l_SRmObj);
                        }

                        // Add to cache and store byte place to memory
                        m_SCache.append(l_SObj);
                        l_SObj->startFrame = m_lCacheFramePos;
                        l_SObj->length = l_iRet;
                        m_lCacheFramePos += l_iRet / (sizeof(CSAMPLE) * 2);

                        // Ogg/Opus have packages pos that have many
                        // audio frames so seek next unique pos..
                        if (l_SPacket.pos != l_lLastPacketPos) {
                            m_bUnique = true;
                            l_lLastPacketPos = l_SPacket.pos;
                        }

                        // If we are over last storepos and we have read more than jump point need is and pos is unique we store it to memory
                        if (m_lCacheFramePos > m_lLastStoredPos &&
                                m_lCacheFramePos > (AUDIOSOURCEFFMPEG_POSDISTANCE + m_lLastStoredPos) &&
                                m_bUnique == true) {
                            struct ffmpegLocationObject  *l_SJmp = (struct ffmpegLocationObject  *)malloc(
                                    sizeof(struct ffmpegLocationObject));
                            m_lLastStoredPos = m_lCacheFramePos;
                            l_SJmp->startFrame = m_lCacheFramePos;
                            l_SJmp->pos = l_SPacket.pos;
                            l_SJmp->pts = l_SPacket.pts;
                            m_SJumpPoints.append(l_SJmp);
                            m_bUnique = false;
                        }

                        if (offset < 0 || offset <= m_lCacheFramePos) {
                            l_iCount --;
                        }
                    } else {
                        free(l_SObj);
                        l_SObj = NULL;
                        qDebug() <<
                                 "SoundSourceFFmpeg::readFramesToCache: General error in audio decode:" <<
                                 l_iRet;
                    }
                }

                av_free_packet(&l_SPacket);
                l_SPacket.data = NULL;
                l_SPacket.size = 0;

            } else {
                l_iError ++;
                if (l_iError == 5) {
                    // Stream end and we couldn't read enough frames
                    l_iStop = true;
                }
            }


        } else {
            qDebug() << "SoundSourceFFmpeg::readFramesToCache: Packet too big or File end";
            l_iStop = true;
        }

    }

    if (l_pFrame != NULL) {
      l_iFrameCount --;
// FFMPEG 2.2 3561060 anb beyond
#if LIBAVCODEC_VERSION_INT >= 3561060
          av_frame_unref(l_pFrame);
          av_frame_free(&l_pFrame);
// FFMPEG 0.11 and below
#elif LIBAVCODEC_VERSION_INT <= 3544932
          av_free(l_pFrame);
// FFMPEG 1.0 - 2.1
#else
          avcodec_free_frame(&l_pFrame);
#endif
          l_pFrame = NULL;

    }

    if (l_iFrameCount > 0) {
       qDebug() << "SoundSourceFFmpeg::readFramesToCache(): Frame balance is not 0 it is: " << l_iFrameCount;
    }

    l_SObj = m_SCache.first();
    m_lCacheStartFrame = l_SObj->startFrame;
    l_SObj = m_SCache.last();
    m_lCacheEndFrame = (l_SObj->startFrame + l_SObj->length);

    if (!l_iCount) {
        return true;
    } else {
        return false;
    }
}

bool SoundSourceFFmpeg::getBytesFromCache(char *buffer, SINT offset,
        SINT size) {
    struct ffmpegCacheObject *l_SObj = NULL;
    quint32 l_lPos = 0;
    quint32 l_lLeft = 0;
    quint32 l_lOffset = 0;
    quint32 l_lBytesToCopy = 0;

    // Is offset bigger than start of cache
    if (offset >= m_lCacheStartFrame) {
        // If last pos is (which is shouldn't) use caches end
        if (m_lCacheLastPos == 0) {
            m_lCacheLastPos = m_SCache.size() - 1;
        }

        // Seek to correct FrameIndex
        for (l_lPos = m_lCacheLastPos; l_lPos > 0; l_lPos --) {
            l_SObj = m_SCache[l_lPos];
            if ((l_SObj->startFrame + l_SObj->length) < offset) {
                break;
            }
        }

        // Use this Cache object as starting point
        l_SObj = m_SCache[l_lPos];

<<<<<<< HEAD
<<<<<<< HEAD
        // Calculate in other words get bytes how much we must copy to
        // buffer (CSAMPLE = 4 and we have 2 channels which is 8 times)
=======
>>>>>>> Fixed native FFmpeg playing with New Sound API whic uses Float point reading.
=======
        // Calculate in other words get bytes how much we must copy to
        // buffer (CSAMPLE = 4 and we have 2 channels which is 8 times)
>>>>>>> Added some comments how caching and FFmpeg/AVConv really works
        l_lLeft = (size * sizeof(CSAMPLE)) * 2;
        memset(buffer, 0x00, l_lLeft);
        while (l_lLeft > 0) {
            // If Cache is running low read more
            if (l_SObj == NULL || (l_lPos + 5) > (unsigned int)m_SCache.size()) {
                offset = l_SObj->startFrame;
                if (readFramesToCache(50, -1) == false) {
                    return false;
                }
                // Seek back to correct place
                for (l_lPos = (m_SCache.size() - 50); l_lPos > 0; l_lPos --) {
                    l_SObj = m_SCache[l_lPos];
                    if ((l_SObj->startFrame + l_SObj->length) < offset) {
                        break;
                    }
                }
                l_SObj = m_SCache[l_lPos];
                continue;
            }

            // If Cache object ain't correct then calculate offset
<<<<<<< HEAD
            if (l_SObj->startByte <= offset) {
<<<<<<< HEAD
<<<<<<< HEAD
                // We have to convert again it to bytes
                l_lOffset = (offset - l_SObj->startByte) * (sizeof(CSAMPLE) * 2);
            }

            // Okay somehow offset is bigger than our Cache object have bytes
=======
                l_lOffset = (offset - l_SObj->startByte) * (sizeof(CSAMPLE) * 2);
            }

>>>>>>> Fixed native FFmpeg playing with New Sound API whic uses Float point reading.
=======
=======
            if (l_SObj->startFrame <= offset) {
>>>>>>> Get rid of Byte word in variables (use Frame) in SoundSourceFFMPEG and change everything to SINT which seems to be preferred type
                // We have to convert again it to bytes
                l_lOffset = (offset - l_SObj->startFrame) * (sizeof(CSAMPLE) * 2);
            }

            // Okay somehow offset is bigger than our Cache object have bytes
>>>>>>> Added some comments how caching and FFmpeg/AVConv really works
            if (l_lOffset >= l_SObj->length) {
                l_SObj = m_SCache[++ l_lPos];
                continue;
            }

            if (l_lLeft > l_SObj->length) {
<<<<<<< HEAD
<<<<<<< HEAD
                // calculate start point of copy
=======
>>>>>>> Fixed native FFmpeg playing with New Sound API whic uses Float point reading.
=======
                // calculate start point of copy
>>>>>>> Added some comments how caching and FFmpeg/AVConv really works
                l_lBytesToCopy = l_SObj->length - l_lOffset;
                memcpy(buffer, (l_SObj->bytes + l_lOffset), l_lBytesToCopy);
                l_lOffset = 0;
                buffer += l_lBytesToCopy;
                l_lLeft -= l_lBytesToCopy;
            } else {
                memcpy(buffer, l_SObj->bytes, l_lLeft);
                l_lLeft = 0;
            }

            l_SObj = m_SCache[++ l_lPos];
        }

        m_lCacheLastPos = --l_lPos;
        return true;
    }

    return false;
}

<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
Result SoundSourceFFmpeg::parseHeader() {
    QString location = getFilename();
    setType(location.section(".",-1).toLower());
=======
Result SoundSourceFFmpeg::parseMetadata(Mixxx::TrackMetadata* pMetadata) {
=======
Result SoundSourceFFmpeg::parseMetadata(Mixxx::TrackMetadata* pMetadata) const {
<<<<<<< HEAD
>>>>>>> Split AudioSource from SoundSource
    qDebug() << "ffmpeg: SoundSourceFFmpeg::parseMetadata" << getFilename();
=======
    qDebug() << "ffmpeg: SoundSourceFFmpeg::parseMetadata" << getLocalFileName();
>>>>>>> Harmonize handling of URL (re-)sources

    AVFormatContext *FmtCtx = avformat_alloc_context();
    AVCodecContext *CodecCtx = NULL;
    AVDictionaryEntry *FmtTag = NULL;
    unsigned int i;
    AVDictionary *l_iFormatOpts = NULL;
>>>>>>> Move track metadata properties from SoundSource into separate DTO class

<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
    QByteArray qBAFilename = getFilename().toLocal8Bit();
=======
    if (avformat_open_input(&FmtCtx, getFilename().toLocal8Bit().constData(), NULL,
=======
    if (avformat_open_input(&FmtCtx, getLocalFilePath().constData(), NULL,
>>>>>>> Create SoundSource from URL
=======
    if (avformat_open_input(&FmtCtx, getLocalFileNameBytes().constData(), NULL,
>>>>>>> Harmonize handling of URL (re-)sources
                            &l_iFormatOpts) !=0) {
        qDebug() << "av_open_input_file: cannot open" << getLocalFileName();
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
                getLocalFileName();
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
                getLocalFileName();
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
            pMetadata->setReplayGain(Mixxx::TrackMetadata::parseReplayGain(strValue));
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
    return Mixxx::AudioSourceFFmpeg::create(getUrl());
=======
SINT SoundSourceFFmpeg::seekSampleFrame(SINT frameIndex) {
    DEBUG_ASSERT(isValidFrameIndex(frameIndex));

    int ret = 0;
    qint64 i = 0;

    if (frameIndex < 0 || frameIndex < m_lCacheStartFrame) {
        ret = avformat_seek_file(m_pFormatCtx,
                                 m_iAudioStream,
                                 0,
                                 32767 * 2,
                                 32767 * 2,
                                 AVSEEK_FLAG_BACKWARD);


        if (ret < 0) {
            qDebug() << "SoundSourceFFmpeg::seek: Can't seek to 0 byte!";
            return -1;
        }

        clearCache();
        m_lCacheStartFrame = 0;
        m_lCacheEndFrame = 0;
        m_lCacheLastPos = 0;
        m_lCacheFramePos = 0;
        m_lStoredSeekPoint = -1;


        // Try to find some jump point near to
        // where we are located so we don't needed
        // to try guess it
        if (frameIndex >= AUDIOSOURCEFFMPEG_POSDISTANCE) {
            for (i = 0; i < m_SJumpPoints.size(); i ++) {
                if (m_SJumpPoints[i]->startFrame >= frameIndex && i > 2) {
                    m_lCacheFramePos = m_SJumpPoints[i - 2]->startFrame * 2;
                    m_lStoredSeekPoint = m_SJumpPoints[i - 2]->pos;
                    break;
                }
            }
        }

        if (frameIndex == 0) {
            readFramesToCache((AUDIOSOURCEFFMPEG_CACHESIZE - 50), -1);
        } else {
            readFramesToCache((AUDIOSOURCEFFMPEG_CACHESIZE / 2), frameIndex);
        }
    }


    if (m_lCacheEndFrame <= frameIndex) {
        readFramesToCache(100, frameIndex);
    }

    m_currentMixxxFrameIndex = frameIndex;

    m_bIsSeeked = TRUE;

    return frameIndex;
>>>>>>> Move code from specialized AudioSources back into corresponding SoundSources
}

SINT SoundSourceFFmpeg::readSampleFrames(SINT numberOfFrames,
        CSAMPLE* sampleBuffer) {

    if (m_SCache.size() == 0) {
        // Make sure we allways start at begining and cache have some
        // material that we can consume.
        seekSampleFrame(0);
        m_bIsSeeked = FALSE;
    }

    getBytesFromCache((char *)sampleBuffer, m_currentMixxxFrameIndex, numberOfFrames);

    //  As this is also Hack
    // If we don't seek like we don't on analyzer.. keep
    // place in mind..
    if (m_bIsSeeked == FALSE) {
        m_currentMixxxFrameIndex += numberOfFrames;
    }

    m_bIsSeeked = FALSE;

    return numberOfFrames;
}

} // namespace Mixxx
