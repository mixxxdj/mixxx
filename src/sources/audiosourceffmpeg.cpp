#include "sources/audiosourceffmpeg.h"

#include <QtDebug>

#include <vector>

#define AUDIOSOURCEFFMPEG_CACHESIZE 1000
#define AUDIOSOURCEFFMPEG_POSDISTANCE ((1024 * 1000) / 8)

namespace Mixxx {

AudioSourceFFmpeg::AudioSourceFFmpeg(QUrl url)
    : AudioSource(url),
      m_pFormatCtx(NULL),
      m_iAudioStream(-1),
      m_pCodecCtx(NULL),
      m_pCodec(NULL),
      m_pResample(NULL),
      m_iCurrentMixxTs(0),
      m_bIsSeeked(false),
      m_lCacheBytePos(0),
      m_lCacheStartByte(0),
      m_lCacheEndByte(0),
      m_lCacheLastPos(0),
      m_lLastStoredPos(0),
      m_lStoredSeekPoint(-1) {
}

AudioSourceFFmpeg::~AudioSourceFFmpeg() {
    preDestroy();
}

AudioSourcePointer AudioSourceFFmpeg::create(QUrl url) {
    return onCreate(new AudioSourceFFmpeg(url));
}

Result AudioSourceFFmpeg::postConstruct() {
    unsigned int i;
    AVDictionary *l_iFormatOpts = NULL;

    const QByteArray qBAFilename(getLocalFileNameBytes());

    qDebug() << "New AudioSourceFFmpeg :" << qBAFilename;

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
    if (m_iAudioStream==-1) {
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
    // TODO(XXX): Use AV_SAMPLE_FMT_FLT instead of AV_SAMPLE_FMT_S16
    m_pResample->open(m_pCodecCtx->sample_fmt, AV_SAMPLE_FMT_S16);

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

void AudioSourceFFmpeg::preDestroy() {
    clearCache();

    if (m_pCodecCtx != NULL) {
        qDebug() << "~AudioSourceFFmpeg(): Clear FFMPEG stuff";
        avcodec_close(m_pCodecCtx);
        avformat_close_input(&m_pFormatCtx);
        av_free(m_pFormatCtx);
        m_pFormatCtx = NULL;
    }

    if (m_pResample != NULL) {
        qDebug() << "~AudioSourceFFmpeg(): Delete FFMPEG Resampler";
        delete m_pResample;
        m_pResample = NULL;
    }

    while (m_SJumpPoints.size() > 0) {
        ffmpegLocationObject* l_SRmJmp = m_SJumpPoints[0];
        m_SJumpPoints.remove(0);
        free(l_SRmJmp);
    }
}

void AudioSourceFFmpeg::clearCache() {
    while (m_SCache.size() > 0) {
        struct ffmpegCacheObject* l_SRmObj = m_SCache[0];
        m_SCache.remove(0);
        free(l_SRmObj->bytes);
        free(l_SRmObj);
    }
}

bool AudioSourceFFmpeg::readFramesToCache(unsigned int count, qint64 offset) {
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

        if (av_read_frame(m_pFormatCtx, &l_SPacket) >= 0) {
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
                        qDebug() << "AudioSourceFFmpeg::readFramesToCache: Not enough memory!";
                        l_iStop = true;
                        continue;
                    }
                    memset(l_SObj, 0x00, sizeof(struct ffmpegCacheObject));
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
                        l_SObj->startByte = m_lCacheBytePos / 2;
                        l_SObj->length = l_iRet / 2;
                        m_lCacheBytePos += l_iRet;

                        // Ogg/Opus have packages pos that have many
                        // audio frames so seek next unique pos..
                        if (l_SPacket.pos != l_lLastPacketPos) {
                            m_bUnique = true;
                            l_lLastPacketPos = l_SPacket.pos;
                        }

                        // If we are over last storepos and we have read more than jump point need is and pos is unique we store it to memory
                        if (m_lCacheBytePos > m_lLastStoredPos &&
                                m_lCacheBytePos > (AUDIOSOURCEFFMPEG_POSDISTANCE + m_lLastStoredPos) &&
                                m_bUnique == true) {
                            struct ffmpegLocationObject  *l_SJmp = (struct ffmpegLocationObject  *)malloc(
                                    sizeof(struct ffmpegLocationObject));
                            m_lLastStoredPos = m_lCacheBytePos;
                            l_SJmp->startByte = m_lCacheBytePos / 2;
                            l_SJmp->pos = l_SPacket.pos;
                            l_SJmp->pts = l_SPacket.pts;
                            m_SJumpPoints.append(l_SJmp);
                            m_bUnique = false;
                        }

                        if (offset < 0 || (quint64) offset <= (m_lCacheBytePos / 2)) {
                            l_iCount --;
                        }
                    } else {
                        free(l_SObj);
                        l_SObj = NULL;
                        qDebug() <<
                                 "AudioSourceFFmpeg::readFramesToCache: General error in audio decode:" <<
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
            qDebug() << "AudioSourceFFmpeg::readFramesToCache: Packet too big or File end";
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
       qDebug() << "AudioSourceFFmpeg::readFramesToCache(): Frame balance is not 0 it is: " << l_iFrameCount;
    }

    l_SObj = m_SCache.first();
    m_lCacheStartByte = l_SObj->startByte;
    l_SObj = m_SCache.last();
    m_lCacheEndByte = (l_SObj->startByte + l_SObj->length);

    if (!l_iCount) {
        return true;
    } else {
        return false;
    }
}

bool AudioSourceFFmpeg::getBytesFromCache(char *buffer, quint64 offset,
        quint64 size) {
    struct ffmpegCacheObject *l_SObj = NULL;
    quint32 l_lPos = 0;
    quint32 l_lLeft = 0;
    quint32 l_lOffset = 0;
    quint32 l_lBytesToCopy = 0;

    if (offset >= m_lCacheStartByte) {
        if (m_lCacheLastPos == 0) {
            m_lCacheLastPos = m_SCache.size() - 1;
        }
        for (l_lPos = m_lCacheLastPos; l_lPos > 0; l_lPos --) {
            l_SObj = m_SCache[l_lPos];
            if ((l_SObj->startByte + l_SObj->length) < offset) {
                break;
            }
        }

        l_SObj = m_SCache[l_lPos];

        l_lLeft = (size * 2);
        memset(buffer, 0x00, l_lLeft);
        while (l_lLeft > 0) {

            if (l_SObj == NULL || (l_lPos + 5) > (unsigned int)m_SCache.size()) {
                offset = l_SObj->startByte;
                if (readFramesToCache(50, -1) == false) {
                    return false;
                }
                for (l_lPos = (m_SCache.size() - 50); l_lPos > 0; l_lPos --) {
                    l_SObj = m_SCache[l_lPos];
                    if ((l_SObj->startByte + l_SObj->length) < offset) {
                        break;
                    }
                }
                l_SObj = m_SCache[l_lPos];
                continue;
            }

            if (l_SObj->startByte <= offset) {
                l_lOffset = (offset - l_SObj->startByte) * 2;
            }

            if (l_lOffset >= (l_SObj->length * 2)) {
                l_SObj = m_SCache[++ l_lPos];
                continue;
            }

            if (l_lLeft > (l_SObj->length * 2)) {
                l_lBytesToCopy = ((l_SObj->length * 2)  - l_lOffset);
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

AudioSource::diff_type AudioSourceFFmpeg::seekSampleFrame(diff_type frameIndex) {
    DEBUG_ASSERT(isValidFrameIndex(frameIndex));

    const diff_type filepos = frames2samples(frameIndex);

    int ret = 0;
    qint64 i = 0;

    if (filepos < 0 || (unsigned long) filepos < m_lCacheStartByte) {
        ret = avformat_seek_file(m_pFormatCtx,
                                 m_iAudioStream,
                                 0,
                                 32767 * 2,
                                 32767 * 2,
                                 AVSEEK_FLAG_BACKWARD);


        if (ret < 0) {
            qDebug() << "AudioSourceFFmpeg::seek: Can't seek to 0 byte!";
            return -1;
        }

        clearCache();
        m_lCacheStartByte = 0;
        m_lCacheEndByte = 0;
        m_lCacheLastPos = 0;
        m_lCacheBytePos = 0;
        m_lStoredSeekPoint = -1;


        // Try to find some jump point near to
        // where we are located so we don't needed
        // to try guess it
        if (filepos >= AUDIOSOURCEFFMPEG_POSDISTANCE) {
            for (i = 0; i < m_SJumpPoints.size(); i ++) {
                if (m_SJumpPoints[i]->startByte >= (unsigned long) filepos && i > 2) {
                    m_lCacheBytePos = m_SJumpPoints[i - 2]->startByte * 2;
                    m_lStoredSeekPoint = m_SJumpPoints[i - 2]->pos;
                    break;
                }
            }
        }

        if (filepos == 0) {
            readFramesToCache((AUDIOSOURCEFFMPEG_CACHESIZE - 50), -1);
        } else {
            readFramesToCache((AUDIOSOURCEFFMPEG_CACHESIZE / 2), filepos);
        }
    }


    if (m_lCacheEndByte <= (unsigned long) filepos) {
        readFramesToCache(100, filepos);
    }

    m_iCurrentMixxTs = filepos;

    m_bIsSeeked = TRUE;

    return frameIndex;
}

unsigned int AudioSourceFFmpeg::read(unsigned long size, SAMPLE* destination) {

    if (m_SCache.size() == 0) {
        // Make sure we allways start at begining and cache have some
        // material that we can consume.
        seekSampleFrame(0);
        m_bIsSeeked = FALSE;
    }

    getBytesFromCache((char *)destination, m_iCurrentMixxTs, size);

    //  As this is also Hack
    // If we don't seek like we don't on analyzer.. keep
    // place in mind..
    if (m_bIsSeeked == FALSE) {
        m_iCurrentMixxTs += size;
    }

    m_bIsSeeked = FALSE;
    return size;
}

AudioSource::size_type AudioSourceFFmpeg::readSampleFrames(size_type numberOfFrames,
        sample_type* sampleBuffer) {
    // This is just a hack that simply reuses existing
    // functionality. Sample data should be resampled
    // directly into AV_SAMPLE_FMT_FLT instead of
    // AV_SAMPLE_FMT_S16!
    typedef std::vector<SAMPLE> TempBuffer;
    TempBuffer tempBuffer(frames2samples(numberOfFrames));
    const size_type readSamples = read(tempBuffer.size(), &tempBuffer[0]);
    for (size_type i = 0; i < readSamples; ++i) {
        sampleBuffer[i] = SAMPLE_clampSymmetric(tempBuffer[i]) / sample_type(SAMPLE_MAX);
    }
    return samples2frames(readSamples);
}

} // namespace Mixxx
