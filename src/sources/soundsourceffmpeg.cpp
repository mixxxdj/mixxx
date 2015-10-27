#include "sources/soundsourceffmpeg.h"

#include <vector>

#define AUDIOSOURCEFFMPEG_CACHESIZE 1000
#define AUDIOSOURCEFFMPEG_MIXXXFRAME_TO_BYTEOFFSET (sizeof(CSAMPLE) * getChannelCount())
#define AUDIOSOURCEFFMPEG_FILL_FROM_CURRENTPOS -1

namespace Mixxx {

QStringList SoundSourceProviderFFmpeg::getSupportedFileExtensions() const {
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
            list.append("mp4");
        } else if (!strcmp(l_SInputFmt->name, "mp4")) {
            list.append("mp4");
        } else if (!strcmp(l_SInputFmt->name, "mp3")) {
            list.append("mp3");
        } else if (!strcmp(l_SInputFmt->name, "aac")) {
            list.append("aac");
        } else if (!strcmp(l_SInputFmt->name, "opus") ||
                   !strcmp(l_SInputFmt->name, "libopus")) {
            list.append("opus");
        } else if (!strcmp(l_SInputFmt->name, "wma") or
                   !strcmp(l_SInputFmt->name, "xwma")) {
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
      m_lStoreCount(0),
      m_lStoredSeekPoint(-1),
      m_SStoredJumpPoint(NULL) {
}

SoundSourceFFmpeg::~SoundSourceFFmpeg() {
    close();
}

Result SoundSourceFFmpeg::tryOpen(const AudioSourceConfig& /*audioSrcCfg*/) {
    unsigned int i;
    AVDictionary *l_iFormatOpts = NULL;

    const QString localFileName(getLocalFileName());
    qDebug() << "New SoundSourceFFmpeg :" << localFileName;

    DEBUG_ASSERT(!m_pFormatCtx);
    m_pFormatCtx = avformat_alloc_context();

    if (m_pFormatCtx == NULL) {
        qDebug() << "SoundSourceFFmpeg::tryOpen: Can't allocate memory";
        return ERR;
    }

    // TODO() why is this required, should't it be a runtime check
#if LIBAVCODEC_VERSION_INT < 3622144 // 55.69.0
    m_pFormatCtx->max_analyze_duration = 999999999;
#endif

    // libav replaces open() with ff_win32_open() which accepts a
    // Utf8 path
    // see: avformat/os_support.h
    // The old method defining an URL_PROTOCOL is deprecated
#if defined(_WIN32) && !defined(__MINGW32CE__)
    const QByteArray qBAFilename(
            avformat_version() >= ((52<<16)+(0<<8)+0) ?
            getLocalFileName().toUtf8() :
            getLocalFileName().toLocal8Bit());
#else
    const QByteArray qBAFilename(getLocalFileName().toLocal8Bit());
#endif

    // Open file and make m_pFormatCtx
    if (avformat_open_input(&m_pFormatCtx, qBAFilename.constData(), NULL,
                            &l_iFormatOpts) != 0) {
        qDebug() << "SoundSourceFFmpeg::tryOpen: cannot open" << localFileName;
        return ERR;
    }

    // TODO() why is this required, should't it be a runtime check
#if LIBAVCODEC_VERSION_INT > 3544932 // 54.23.100
    av_dict_free(&l_iFormatOpts);
#endif

    // Retrieve stream information
    if (avformat_find_stream_info(m_pFormatCtx, NULL) < 0) {
        qDebug() << "SoundSourceFFmpeg::tryOpen: cannot open" << localFileName;
        return ERR;
    }

    //debug only (Enable if needed)
    //av_dump_format(m_pFormatCtx, 0, qBAFilename.constData(), false);

    // Find the first audio stream
    m_iAudioStream = -1;

    for (i = 0; i < m_pFormatCtx->nb_streams; i++)
        if (m_pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            m_iAudioStream = i;
            break;
        }

    if (m_iAudioStream == -1) {
        qDebug() <<
                 "SoundSourceFFmpeg::tryOpen: cannot find an audio stream: cannot open"
                 << localFileName;
        return ERR;
    }

    // Get a pointer to the codec context for the audio stream
    m_pCodecCtx = m_pFormatCtx->streams[m_iAudioStream]->codec;

    // Find the decoder for the audio stream
    if (!(m_pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id))) {
        qDebug() << "SoundSourceFFmpeg::tryOpen: cannot find a decoder for" <<
                localFileName;
        return ERR;
    }

    if (avcodec_open2(m_pCodecCtx, m_pCodec, NULL)<0) {
        qDebug() << "SoundSourceFFmpeg::tryOpen: cannot open" << localFileName;
        return ERR;
    }

    m_pResample = new EncoderFfmpegResample(m_pCodecCtx);
    m_pResample->openMixxx(m_pCodecCtx->sample_fmt, AV_SAMPLE_FMT_FLT);

    setChannelCount(m_pCodecCtx->channels);
    setSamplingRate(m_pCodecCtx->sample_rate);
    setFrameCount((qint64)round((double)((double)m_pFormatCtx->duration *
                                         (double)m_pCodecCtx->sample_rate) / (double)AV_TIME_BASE));

    qDebug() << "SoundSourceFFmpeg::tryOpen: Sampling rate: " << getSamplingRate() <<
             ", Channels: " <<
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

        if (l_pFrame == NULL) {
            qDebug() << "SoundSourceFFmpeg::readFramesToCache: Can't alloc memory!";
            return false;
        }

        // Read one frame (which has nothing to do with Mixxx Frame)
        // it's some packed audio data from container like MP3, Ogg or MP4
        if (av_read_frame(m_pFormatCtx, &l_SPacket) >= 0) {
            // Are we on correct audio stream. Currently we are always
            // Using first audio stream but in future there should be
            // possibility to choose which to use
            if (l_SPacket.stream_index == m_iAudioStream &&
                    l_SPacket.pos >= 0) {
                if (m_lStoredSeekPoint > 0) {
                    struct ffmpegLocationObject *l_STestObj = NULL;
                    if (m_SJumpPoints.size() > 0) {
                        l_STestObj = m_SJumpPoints.first();

                        if (l_STestObj->pos > l_SPacket.pos) {
                            continue;
                        }
                    }

                    // Seek for correct jump point
                    if (m_lStoredSeekPoint > l_SPacket.pos) {
                        av_free_packet(&l_SPacket);
                        l_SPacket.data = NULL;
                        l_SPacket.size = 0;
                        continue;
                    }
                    m_lStoredSeekPoint = -1;
                    m_SStoredJumpPoint = NULL;
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
                    l_iRet = m_pResample->reSampleMixxx(l_pFrame, &l_SObj->bytes);

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
                        m_lCacheFramePos += l_iRet / AUDIOSOURCEFFMPEG_MIXXXFRAME_TO_BYTEOFFSET;

                        // Ogg/Opus have packages pos that have many
                        // audio frames so seek next unique pos..
                        if (l_SPacket.pos != l_lLastPacketPos) {
                            l_lLastPacketPos = l_SPacket.pos;
                            m_lStoreCount ++;
                        }

                        // If we are over last storepos and we have read more than jump point need is
                        // and pos is unique we store it to memory
                        //
                        // So why number 32? Sorry I just made that up from the hat there is no
                        // math behind it. Number 32 it's not too big nor jumps points are not
                        // too close each other. Mainly it's ugly compromise with MP3,MP4,OGG and WMA
                        // diffrent codec frame sizes
                        if (m_lStoreCount == 32) {
                            struct ffmpegLocationObject *l_STestObj = NULL;

                            if (m_SJumpPoints.size() > 0) {
                                l_STestObj = m_SJumpPoints.last();
                            }
                            // Check wether we have this jumppoint stored allready or not
                            // We should have jumppoints below that pos
                            if (l_STestObj == NULL || l_STestObj->pos < l_SPacket.pos) {
                                struct ffmpegLocationObject  *l_SJmp = (struct ffmpegLocationObject  *)malloc(
                                        sizeof(struct ffmpegLocationObject));
                                m_lLastStoredPos = m_lCacheFramePos;
                                l_SJmp->startFrame = m_lCacheFramePos;
                                l_SJmp->pos = l_SPacket.pos;
                                l_SJmp->pts = l_SPacket.pts;
                                m_SJumpPoints.append(l_SJmp);
                            }
                            m_lStoreCount = 0;
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
        qDebug() <<
                 "SoundSourceFFmpeg::readFramesToCache(): Frame balance is not 0 it is: " <<
                 l_iFrameCount;
    }

    if (m_SCache.isEmpty()) {
        qDebug() << "SoundSourceFFmpeg::readFramesToCache(): Can't read frames. Cache empty!";
        return false;
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

bool SoundSourceFFmpeg::getBytesFromCache(CSAMPLE* buffer, SINT offset,
        SINT size) {
    struct ffmpegCacheObject *l_SObj = NULL;
    qint32 l_lPos = 0;
    quint32 l_lLeft = size * AUDIOSOURCEFFMPEG_MIXXXFRAME_TO_BYTEOFFSET;
    quint32 l_lOffset = 0;
    quint32 l_lBytesToCopy = 0;
    bool l_bEndOfFile = false;

    char *l_pBuffer = (char *)buffer;

    // If cache is empty then retun without crash.
    if (m_SCache.isEmpty()) {
        qDebug() << "SoundSourceFFmpeg::getBytesFromCache: Cache is empty can't return bytes";
        memset(l_pBuffer, 0x00, l_lLeft);
        return false;
    }

    // Is offset bigger than start of cache
    if (offset >= m_lCacheStartFrame) {
        int l_lTmpLen = 0;
        // If last pos is (which is shouldn't) use caches end
        if (m_lCacheLastPos == 0) {
            m_lCacheLastPos = m_SCache.size() - 1;
        }

        // Seek to correct FrameIndex (Minus 5 for faster seek)
        //
        // This could be done per steps but because there
        // Jump points can be far away and codec frames are small
        // just jump to point where is safe to start.
        for (l_lPos = m_lCacheLastPos; l_lPos >= 0; l_lPos -= 5) {
            l_SObj = m_SCache[l_lPos];

            // Because length is in byte we have to convert it to Frames
            l_lTmpLen = l_SObj->length / AUDIOSOURCEFFMPEG_MIXXXFRAME_TO_BYTEOFFSET;

            if ((l_SObj->startFrame + l_lTmpLen) < offset) {
                break;
            }
        }

        // Because we step 5 backward we can end up to below zero
        // We can't go futher so hope for the best
        if (l_lPos < 0) {
            l_lPos = 0;
        }

        // This shouldn't never happen.. because it's nearly imposible
        // but because it can happen double check
        if (l_lPos >= m_SCache.size()) {
            l_lPos = m_SCache.size() - 1;
        }

        // Use this Cache object as starting point
        l_SObj = m_SCache[l_lPos];
        
        if (l_SObj == NULL) {
            qDebug() << "SoundSourceFFmpeg::getBytesFromCache: Cache object NULL";
            return false;
        }

        if (l_pBuffer == NULL) {
            qDebug() << "SoundSourceFFmpeg::getBytesFromCache: Out buffer NULL";
            return false;
        }

        while (l_lLeft > 0) {
            // If Cache is running low read more
            if ((l_lPos + 5) > m_SCache.size() && l_bEndOfFile == false) {
                offset = l_SObj->startFrame;
                // Read 50 frames from current pos. If we hit file end before that
                // exit
                if (readFramesToCache(50, AUDIOSOURCEFFMPEG_FILL_FROM_CURRENTPOS) == false) {
                    // File has ended.. don't try to cache anymore
                    // or some fatal error has occurred so.. just don't
                    l_bEndOfFile = true;
                }

                // Seek back to correct place
                for (l_lPos = (m_SCache.size() - 50); l_lPos > 0; l_lPos --) {
                    l_SObj = m_SCache[l_lPos];
                    if ((l_SObj->startFrame + l_SObj->length) < offset) {
                        break;
                    }
                }

                if (l_lPos < m_SCache.size() && l_lPos >= 0) {
                    l_SObj = m_SCache[l_lPos];
                    continue;
                } else if (l_lPos < 0) {
                    l_lPos = 0;
                } else {
                    l_SObj = m_SCache.last();
                    l_lPos = l_lPos < m_SCache.size() - 1;
                }
            }

            // If Cache object ain't correct then calculate offset
            if (l_SObj->startFrame <= offset) {
                // We have to convert again it to bytes
                l_lOffset = (offset - l_SObj->startFrame) *
                            AUDIOSOURCEFFMPEG_MIXXXFRAME_TO_BYTEOFFSET;
            }

            // Okay somehow offset is bigger than our Cache object have bytes
            if (l_lOffset >= l_SObj->length) {
                if ((l_lPos + 1) < m_SCache.size()) {
                    l_SObj = m_SCache[++ l_lPos];
                    continue;
                } else {
                    qDebug() <<
                             "SoundSourceFFmpeg::getBytesFromCache: Buffer run out. Shouldn't happen!";
                    memset(l_pBuffer, 0x00, l_lLeft);
                    return false;
                }
            }

            // If bytes left is bigger than FFmpeg frame bytes available
            // then copy to buffer end and then jump to next FFmpeg frame
            // to understand this here are some examples
            //   * MP3 have size 2304 * 4
            //   * OGG/Opus size 256 - 1024
            //   * WMA size 32767 - 131070
            // and all these are separated in packets nor solid stream of bytes
            // that just can be copied to buffer
            // so that's why this kind of abstraction is needed
            if (l_lLeft > (l_SObj->length - l_lOffset)) {
                // calculate start point of copy
                l_lBytesToCopy = l_SObj->length - l_lOffset;
                memcpy(l_pBuffer, (l_SObj->bytes + l_lOffset), l_lBytesToCopy);
                l_lOffset = 0;
                l_pBuffer += l_lBytesToCopy;
                l_lLeft -= l_lBytesToCopy;
            } else {
                memcpy(l_pBuffer, (l_SObj->bytes + l_lOffset), l_lLeft);
                l_lLeft = 0;
            }

            // If we have more items of cache use them
            // or after that just zero buffer..
            if ((l_lPos + 1) < m_SCache.size()) {
                l_SObj = m_SCache[++ l_lPos];
            } else {
                // With MP3 VBR length of audio is just a guess
                // it's near good as it can get but it can be too long
                // so fill buffer with 0x00 (zero) that we don't get ugly
                // noise at the end of the file
                memset(l_pBuffer, 0x00, l_lLeft);
                l_lLeft = 0;
            }
        }

        m_lCacheLastPos = --l_lPos;
        return true;
    }

    return false;
}

SINT SoundSourceFFmpeg::seekSampleFrame(SINT frameIndex) {
    DEBUG_ASSERT(isValidFrameIndex(frameIndex));

    int ret = 0;
    qint64 i = 0;
    struct ffmpegLocationObject *l_STestObj = NULL;

    if (frameIndex < 0 || frameIndex < m_lCacheStartFrame) {
        // Seek to set (start of the stream which is FFmpeg frame 0)
        // because we are dealing with compressed audio FFmpeg takes
        // best of to seek that point (in this case 0 Is allways there)
        // in every other case we should provide MIN and MAX tolerance
        // which we can take.
        // FFmpeg just just can't take zero as MAX tolerance so we try to
        // just make some tolerable (which is never used because zero point
        // should always be there) some number (which is 0xffff 65535)
        // that is chosen because in WMA frames can be that big and if it's
        // smaller than the frame we are seeking we can get into error
        ret = avformat_seek_file(m_pFormatCtx,
                                 m_iAudioStream,
                                 0,
                                 0,
                                 0xffff,
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
        if (m_SJumpPoints.size() > 0) {
            l_STestObj = m_SJumpPoints.first();

            if (frameIndex > l_STestObj->startFrame) {
                for (i = 0; i < m_SJumpPoints.size(); i ++) {
                    if (m_SJumpPoints[i]->startFrame >= frameIndex) {
                        if (i > 0) {
                            i --;
                        }

                        m_lCacheFramePos = m_SJumpPoints[i]->startFrame;
                        m_lStoredSeekPoint = m_SJumpPoints[i]->pos;
                        m_SStoredJumpPoint = m_SJumpPoints[i];
                        break;
                    }
                }
            }
        }

        if (frameIndex == 0) {
            // Because we are in the begining just read cache full
            // but leave 50 of just in case
            // -1 one means we are seeking from current position and
            // filling the cache
            readFramesToCache((AUDIOSOURCEFFMPEG_CACHESIZE - 50),
                              AUDIOSOURCEFFMPEG_FILL_FROM_CURRENTPOS);
        }
    }

    if (m_lCacheEndFrame <= frameIndex) {
        // Cache tries to read until it gets to frameIndex
        // after that we still read 100 FFmpeg frames to memory
        // so we have good cache to go forward (100) and backward (900)
        // from the point
        readFramesToCache(100, frameIndex);
    }

    m_currentMixxxFrameIndex = frameIndex;

    m_bIsSeeked = true;

    return frameIndex;
}

SINT SoundSourceFFmpeg::readSampleFrames(SINT numberOfFrames,
        CSAMPLE* sampleBuffer) {

    if (m_SCache.size() == 0) {
        // Make sure we allways start at begining and cache have some
        // material that we can consume.
        seekSampleFrame(0);
        m_bIsSeeked = false;
    }

    getBytesFromCache(sampleBuffer, m_currentMixxxFrameIndex, numberOfFrames);

    //  As this is also Hack
    // If we don't seek like we don't on analyzer.. keep
    // place in mind..
    if (m_bIsSeeked == false) {
        m_currentMixxxFrameIndex += numberOfFrames;
    }

    m_bIsSeeked = false;

    return numberOfFrames;
}

} // namespace Mixxx
