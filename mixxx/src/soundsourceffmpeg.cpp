/* -*- mode:C++; indent-tabs-mode:t; tab-width:8; c-basic-offset:4; -*- */
/***************************************************************************
                          soundsourceffmpeg.cpp -  ffmpeg decoder
                             -------------------
    copyright            : (C) 2007 by Cedric GESTES
    email                : ctaf42@gmail.com
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "trackinfoobject.h"
#include "soundsourceffmpeg.h"
//#ifdef __WINDOWS__
//#include <io.h>
//#include <fcntl.h>
//#endif

// GED's notes - 2007-09-09
// sudo aptitude install libavcodec-dev libavformat-dev liba52-dev libdts-dev
// add '#include <QDebug> to fix qDebug' << "stuff" syntax.
#include <QDebug>

static QMutex ffmpegmutex;
static bool ffmpeginit = false;

/* TODO
 * - seek doesnt work correctly (seek is not precise for mp3)
 * - remove lock except for av_open/av_close
 * - do something like in soundbuffermp3 (keep a list of all frame for seeking)
 * - make ffmpeginit a singleton
 * DTS is not always updated, (maybe only after an av_seek_frame)
 * - remove qdebug (it slow down mixxx a lot)
 */

static void FFmpegInit()
{
    if (!ffmpeginit) {
        qDebug() << "Initialising avcodec/avformat";
        av_register_all();
        ffmpeginit = true;
    }
}

//TODO: handle error
SoundSourceFFmpeg::SoundSourceFFmpeg(QString qFilename) : SoundSource(qFilename)
{
    AVFormatParameters param;
    int i;
    QByteArray fname;

    packet.data = NULL;
    bufferOffset = 0;
    bufferSize = 0;
    memset(buffer, 0, AVCODEC_MAX_AUDIO_FRAME_SIZE);
    fname = qFilename.toLatin1();
    FFmpegInit();

    qDebug() << "New SoundSourceFFmpeg :" << fname;

    /* initialize param to something so av_open_input_file works for raw */
    memset(&param, 0, sizeof(AVFormatParameters));
    param.channels = 2;
    param.sample_rate = 44100;

    iformat = av_find_input_format(fname.constData());
    // Open audio file
    if(av_open_input_file(&pFormatCtx, fname.constData(), iformat, 0, &param)!=0) {
        qDebug() << "av_open_input_file: cannot open" << fname;
        return;
    }

    // Retrieve stream information
    if(av_find_stream_info(pFormatCtx)<0) {
        qDebug() << "av_find_stream_info: cannot open" << fname;
        return;
    }
    //debug only
    dump_format(pFormatCtx, 0, fname.constData(), false);

    qDebug() << "ffmpeg: using the first audio stream available";
    // Find the first video stream
    audioStream=-1;
    for(i=0; i<pFormatCtx->nb_streams; i++)
        if(pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_AUDIO) {
            audioStream=i;
            break;
        }
    if(audioStream==-1) {
        qDebug() << "cannot find an audio stream: cannot open" << fname;
        return;
    }

    // Get a pointer to the codec context for the video stream
    pCodecCtx=pFormatCtx->streams[audioStream]->codec;

    // Find the decoder for the audio stream
    if(!(pCodec=avcodec_find_decoder(pCodecCtx->codec_id))) {
        qDebug() << "cannot find a decoder for" << fname;
        return;
    }

    qDebug() << "ffmpeg: opening the audio codec";
    //avcodec_open is not thread safe
    lock();
    if(avcodec_open(pCodecCtx, pCodec)<0) {
        qDebug() << "avcodec: cannot open" << fname;
        return;
    }
    unlock();

    pFrame=avcodec_alloc_frame();
    channels = pCodecCtx->channels;
    SRATE = pCodecCtx->sample_rate;

    qDebug() << "Samplerate: " << SRATE << ", Channels: " << channels << "\n";
    if(channels > 2){
        qDebug() << "ffmpeg: No support for more than 2 channels!";
        return;
    }
    filelength = (long int) ((double)pFormatCtx->duration * 2 / AV_TIME_BASE * SRATE);

    qDebug() << "ffmpeg: filelength: " << filelength << "d -|- duration: " << pFormatCtx->duration << "ld -- starttime: " << pFormatCtx->streams[audioStream]->start_time << "ld -- " << AV_TIME_BASE << " " << pFormatCtx->streams[audioStream]->codec_info_duration << "ld";
}

SoundSourceFFmpeg::~SoundSourceFFmpeg()
{
    av_free(pFrame);
    // Close the codec
    lock();
    avcodec_close(pCodecCtx);
    unlock();
    // Close the video file
    av_close_input_file(pFormatCtx);
};

void SoundSourceFFmpeg::lock()
{
    //  qDebug() << "ffmpeg: Before lock";
    ffmpegmutex.lock();
    //qDebug() << "ffmpeg: After lock";
}

void SoundSourceFFmpeg::unlock()
{
    //qDebug() << "ffmpeg: Before unlock";
    ffmpegmutex.unlock();
    //qDebug() << "ffmpeg: After unlock";
}

long SoundSourceFFmpeg::ffmpeg2mixxx(long pos, const AVRational &time_base) {
    return (long)((double)pos / (double)time_base.den * (double)SRATE * (double)2.);
}

long SoundSourceFFmpeg::mixxx2ffmpeg(long pos, const AVRational &time_base) {
    return (long)((double)pos / (double)SRATE / (double)2. * (double)time_base.den);
}


/* PLAYGROUND */
/*
   REAL:
   speedfreak
   Debug: file length 28263168
   nofreestyle
   Debug: file length 23904000


   speedfreak
   ratio 1,014428253
   Debug: ffmpeg: filelength: 28427212 -|- duration: 322304000 -- starttime: 0 -- 1000000
   Debug: ffmpeg: Seek ERRORRRRRRRRr ret(-1) filepos(28837367).
   Debug: file length 28427212
   //audiostream=-1
   Debug: ffmpeg: filelength: 28427212 -|- duration: 322304000 -- starttime: 0 -- 1000000
   time base is 1/90000
   Debug: ffmpeg: Seek ERRORRRRRRRRr ret(-1) filepos(320415184).
   --
   Debug: ffmpeg: filelength: 10342873 -|- duration: 117266144 -- starttime: 0 -- 1000000
   Debug: ffmpeg: Seek ERRORRRRRRRRr ret(-1) filepos(10551289).
   Debug: file length 10342873
   seek to <filepos>

   no free style
   Debug: ffmpeg: filelength: 53174016 -|- duration: 602880000 -- starttime: 0 -- 1000000
   time base is 1/90000
   Debug: ffmpeg: Seek ERRORRRRRRRRr ret(-1) filepos(24389275).
   //audistream = -1:
   Debug: ffmpeg: filelength: 53174016 -|- duration: 602880000 -- starttime: 0 -- 1000000
   time base is 1/90000
   Debug: ffmpeg: Seek ERRORRRRRRRRr ret(-1) filepos(270991939).
 */
/*
   int 	avcodec_decode_audio (AVCodecContext *avctx, int16_t *samples, int *frame_size_ptr, uint8_t *buf, int buf_size)
   int 	av_seek_frame (AVFormatContext *s, int stream_index, int64_t timestamp, int flags)
   Seek to the key frame at timestamp.
   Decode an audio frame.
   int 	av_seek_frame_binary (AVFormatContext *s, int stream_index, int64_t target_ts, int flags)
   Does a binary search using av_index_search_timestamp() and AVCodec.read_timestamp().  */

//   secs = filepos / SRATE / 2;
//   mins = secs / 60;
//   secs %= 60;
//   hours = mins / 60;
//   mins %= 60;
/*for (fspos = 0; fspos < 602890000; fspos++){
   ret = av_seek_frame(pFormatCtx, -1, fspos, 0);
   if (ret){
   qDebug() << "ffmpeg: Seek ERRORRRRRRRRr ret(" << ret << ") filepos(" << fspos << "d).";
   fspos--;
   ret = av_seek_frame(pFormatCtx, audioStream, fspos, 0);
   readInput();
   qDebug() << "ffmpeg: seek2EROR " << bufferOffset << " " << bufferSize << " " << packet.pos << "ld";
   return 0;
   }
   }*/
//  std::cout<< "time base is " << time_base.num << "/" << time_base.den << "\n";
//  fspos = (long)((double)filepos / SRATE / 2 * AV_TIME_BASE);

//  fspos2 = (long)((double)filepos / SRATE / 2 * time_base.den);

long SoundSourceFFmpeg::seek(long filepos)
{
    int ret = 0;
    int hours, mins, secs;
    long fspos, diff;
    AVRational time_base = pFormatCtx->streams[audioStream]->time_base;

    lock();

    fspos = mixxx2ffmpeg(filepos, time_base);
    //  qDebug() << "ffmpeg: seek0.5 " << packet.pos << "ld -- " << packet.duration << " -- " << pFormatCtx->streams[audioStream]->cur_dts << "ld";
    qDebug() << "ffmpeg: seek (ffpos " << fspos << "d) (mixxxpos " << filepos << "d)";

    ret = av_seek_frame(pFormatCtx, audioStream, fspos, AVSEEK_FLAG_BACKWARD /*AVSEEK_FLAG_ANY*/);

    if (ret){
        qDebug() << "ffmpeg: Seek ERROR ret(" << ret << ") filepos(" << filepos << "d).";
        unlock();
        return 0;
    }

    readInput();
    diff = ffmpeg2mixxx(fspos - pFormatCtx->streams[audioStream]->cur_dts, time_base);
    qDebug() << "ffmpeg: seeked (dts " << pFormatCtx->streams[audioStream]->cur_dts << ") (diff " << diff << ") (diff " << fspos - pFormatCtx->streams[audioStream]->cur_dts << ")";

    bufferOffset = 0; //diff;
    if (bufferOffset > bufferSize) {
        qDebug() << "ffmpeg: ERROR BAD OFFFFFFSET, buffsize: " << bufferSize << " offset: " << bufferOffset;
        bufferOffset = 0;
    }
    unlock();
    return filepos;
}

/*
 * internal function to only read one paquet
 */
bool SoundSourceFFmpeg::readInput(){
    char * dst;
    unsigned char * src;
    int ret = 0;
    int readsize = 0;
    int inputsize = 0;
    int tries = 0;
    //DEBUG
    bufferSize = 0;
    bufferOffset = 0;
    memset(buffer, 0, AVCODEC_MAX_AUDIO_FRAME_SIZE);
    while (av_read_packet(pFormatCtx, &packet)>0) {
        if (packet.stream_index==audioStream){
            dst = (char *)buffer;
            src = packet.data;
            inputsize = 0;
            readsize = 0;
            //qDebug() << "ffmpeg: before avcodec_decode_audio packet.size(" << packet.size << ")";
            tries = 0;
            do {
                ret = avcodec_decode_audio(pCodecCtx, (int16_t *)dst, &readsize, src, packet.size - inputsize);
                if (readsize == 0)
                {
                    tries++;
                    //qDebug() << "ffmpeg: skip frame, decoded readsize = 0";
                    break;
                }
                if (ret <= 0)
                {
                    tries++;
                    //qDebug() << "ffmpeg: skip frame, decoded ret = 0";
                    if (tries > 3) break;
                    continue;
                }
                dst += readsize;
                bufferSize += readsize;
                src += ret;
                inputsize += ret;
                //qDebug() << "ffmpeg: loop buffersize(" << bufferSize << "), readsize(" << readsize << ") ret(" << ret << ") psize(" << packet.size << ")";
            } while (inputsize < packet.size);
            //qDebug() << "ffmpeg: after avcodec_decode_audio outsize(" << bufferSize << ") - ret(" << ret << ")";
            if (bufferSize != 0)
                return true;

        }
        //debug
        av_free_packet(&packet);
    }
    return false;
}

/*
   read <size> samples into <destination>, and return the number of
   samples actually read.
 */
unsigned SoundSourceFFmpeg::read(unsigned long size, const SAMPLE * destination)
{

    qDebug() << "This code has a bug! It needs fixing before you use it.";
    char * dest = (char *) destination;
    char * src = NULL;
    int index = 0;
    int outsize = 0;

    // rryan 2/2009 This is wrong! read()'s semantics are that
    // destination has only 'size' free items.
    int needed = size*2; //*channels;

    lock();
    qDebug() << "ffmpeg: read, requested:(" << needed / 2 << ")  dts:" << pFormatCtx->streams[audioStream]->cur_dts << "ld buffoffset:" << bufferOffset << " buffsize: " << bufferSize << "\n";
    //copy previous buffer
    src = (char *)buffer;
    src += bufferOffset;
    while (needed > 0) {
        if (bufferOffset < bufferSize) {
            index = bufferSize - bufferOffset > needed ? needed : bufferSize - bufferOffset;
            //qDebug() << "ffmpeg: copy(" << index << ") needed(" << needed << ")";
            memcpy((char *)dest, (char *)(src), index);
            src += index;
            dest += index; //(SAMPLE *)((char *)(dest) + index);
            needed -= index;
            bufferOffset += index;
            outsize += index;
        }
        if (needed > 0 && (bufferSize - bufferOffset <= 0)) {
            bufferOffset = 0;
            readInput();
            src = (char *)buffer;
            src += bufferOffset;
        }
    }
    // convert into stereo if file is mono
    /*if (channels == 1) {
       for(int i=index;i>0;i--) {
       dest[i*2]     = dest[i];
       dest[(i*2)+1] = dest[i];
       }
       }*/
    // return the number of samples in buffer
    unlock();
    return (outsize/2);
}

/*
   Parse the the file to get metadata
 */

int SoundSourceFFmpeg::ParseHeader( TrackInfoObject * Track )
{
    QString location = Track->getLocation();
    AVFormatContext * FmtCtx;
    AVCodecContext * CodecCtx;
    int i, audioStream = -1;
    QByteArray fname;

    fname = location.toAscii();
    FFmpegInit();
    qDebug() << "ffmpeg: pqrsing file:" << fname;
    if(av_open_input_file(&FmtCtx, fname.constData(), NULL, 0, NULL)!=0)
    {
        qDebug() << "av_open_input_file: cannot open" << fname;
        return ERR;
    }
    // Retrieve stream information
    if(av_find_stream_info(FmtCtx)<0)
    {
        qDebug() << "av_find_stream_info: cannot open" << fname;
        return ERR;
    }
    for(i=0; i<FmtCtx->nb_streams; i++)
        if(FmtCtx->streams[i]->codec->codec_type==CODEC_TYPE_AUDIO)
        {
            audioStream=i;
            break;
        }
    if(audioStream==-1)
    {
        qDebug() << "cannot find an audio stream: cannot open" << location;
        return ERR;
    }
    // Get a pointer to the codec context for the video stream
    CodecCtx=FmtCtx->streams[audioStream]->codec;
    Track->setType(location.section(".",-1).toLower());
    Track->setDuration(FmtCtx->duration / AV_TIME_BASE);
    Track->setBitrate((int)(CodecCtx->bit_rate / 1000));
    Track->setSampleRate(CodecCtx->sample_rate);
    Track->setChannels(CodecCtx->channels);
    av_close_input_file(FmtCtx);
    return OK;
}

/*
   Return the length of the file in samples.
 */
inline long unsigned SoundSourceFFmpeg::length()
{
    return filelength;
}
