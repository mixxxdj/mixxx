/****************************************************************************
                   encodermp3.h  - mp3 encoder for mixxx
                             -------------------
    copyright            : (C) 2007 by Wesley Stessens
                           (C) 2009 by Phillip Whelan (rewritten for mp3)
                           (C) 2010 by Tobias Rafreider (fixes for broadcast, dynamic loading of lame_enc.dll, etc)
 ***************************************************************************/

#ifndef ENCODERMP3_H
#define ENCODERMP3_H

#include <QLibrary>

#include "util/types.h"
#include "encoder/encoder.h"
#include "track/track.h"

class EncoderCallback;

class EncoderMp3 : public Encoder {
  public:
    static const int MONO_BITRATE_TRESHOLD;
    static const int MONO_VBR_THRESHOLD;
    static const int MONO_VBR_OFFSET;

    EncoderMp3(EncoderCallback* callback=NULL);
    virtual ~EncoderMp3();

    int initEncoder(int samplerate, QString errorMessage) override;
    void encodeBuffer(const CSAMPLE *samples, const int size) override;
    void updateMetaData(const QString& artist, const QString& title, const QString& album) override;
    void flush() override;
    void setEncoderSettings(const EncoderSettings& settings) override;

  private:
    void initStream();
    int bufferOutGrow(int size);
    int bufferInGrow(int size);

    // For lame
    struct lame_global_struct;
    typedef struct lame_global_struct lame_global_flags;
    typedef lame_global_flags *lame_t;
    lame_global_flags *m_lameFlags;
    
    
    typedef enum vbr_mode_e {
        vbr_off=0,
        vbr_mt,               /* obsolete, same as vbr_mtrh */
        vbr_rh,
        vbr_abr,
        vbr_mtrh,
        vbr_max_indicator,    /* Don't use this! It's used for sanity checks.       */
        vbr_default=vbr_mtrh    /* change this to change the default VBR mode of LAME */
    } vbr_mode;

    /* MPEG modes */
    typedef enum MPEG_mode_e {
        STEREO = 0,
        JOINT_STEREO,
        DUAL_CHANNEL,   /* LAME doesn't supports this! */
        MONO,
        NOT_SET,
        MAX_INDICATOR   /* Don't use this! It's used for sanity checks. */
    } MPEG_mode;

    //Function pointer for lame
    typedef lame_global_flags* (*lame_init__)(void);
    typedef int (*lame_set_num_channels__)(lame_global_flags *, int);
    typedef int (*lame_set_in_samplerate__)(lame_global_flags *, int);
    typedef int (*lame_set_out_samplerate__)(lame_global_flags *, int);
    typedef int (*lame_set_brate__)(lame_global_flags *, int);
    typedef int (*lame_set_mode__)(lame_global_flags *, MPEG_mode);
    typedef int (*lame_set_quality__)(lame_global_flags *, int);
    typedef int (*lame_set_bWriteVbrTag__)(lame_global_flags *, int);
    typedef int (*lame_init_params__)(lame_global_flags *);
    typedef int (*lame_close__)(lame_global_flags *);
    typedef int (*lame_encode_flush__)(
        lame_global_flags *  gfp,               /* global context handle                 */
        unsigned char*       mp3buf,            /* pointer to encoded MP3 stream         */
        int                  size);             /* number of valid octets in this stream */
    typedef int (*lame_encode_buffer_float__)(
        lame_global_flags*  gfp,                /* global context handle         */
        const float         buffer_l [],        /* PCM data for left channel     */
        const float         buffer_r [],        /* PCM data for right channel    */
        const int           nsamples,           /* number of samples per channel */
        unsigned char*      mp3buf,             /* pointer to encoded MP3 stream */
        const int           mp3buf_size );

    // Types of VBR.  default = vbr_off = CBR
    typedef int (*lame_set_VBR__)(lame_global_flags *, vbr_mode);

    // VBR quality level.  0=highest  9=lowest
    typedef int (*lame_set_VBR_q__)(lame_global_flags *, int);

    // Since lame 3.98, else lame_set_VBR_q. Range [0..9.999] (four decimals)
    typedef int (*lame_set_VBR_quality__)(lame_global_flags *, float);

    // Ignored except for VBR=vbr_abr (ABR mode)
    typedef int (*lame_set_VBR_mean_bitrate_kbps__)(lame_global_flags *, int);

    // as lame_encode_buffer_float with +/- 1 full scale and interleaved
    // These methods are present only in lame 3.99, so it's important to test for their presence
    // and fallback to the non-ieee one if not present.
    typedef int (*lame_encode_buffer_interleaved_ieee_float__)(
            lame_t          gfp,
            const float     pcm[],             /* PCM data for left and right */
                                               /* channel, interleaved        */
            const int       nsamples,
            unsigned char * mp3buf,
            const int       mp3buf_size);
    typedef size_t (*lame_get_lametag_frame__)(
        const lame_global_flags *, unsigned char* buffer, size_t size);

    lame_init__                         lame_init;
    lame_set_num_channels__             lame_set_num_channels;
    lame_set_in_samplerate__            lame_set_in_samplerate;
    lame_set_out_samplerate__           lame_set_out_samplerate;
    lame_set_brate__                    lame_set_brate;
    lame_set_mode__                     lame_set_mode;
    lame_set_quality__                  lame_set_quality;
    lame_set_bWriteVbrTag__             lame_set_bWriteVbrTag;
    lame_init_params__                  lame_init_params;
    lame_close__                        lame_close;
    lame_encode_flush__                 lame_encode_flush;
    lame_encode_buffer_float__          lame_encode_buffer_float;
    lame_set_VBR__                      lame_set_VBR;
    lame_set_VBR_q__                    lame_set_VBR_q;
    lame_set_VBR_quality__              lame_set_VBR_quality;
    lame_set_VBR_mean_bitrate_kbps__    lame_set_VBR_mean_bitrate_kbps;
    lame_encode_buffer_interleaved_ieee_float__ lame_encode_buffer_interleaved_ieee_float;
    lame_get_lametag_frame__            lame_get_lametag_frame;

    // Function pointers for ID3 Tags
    typedef void (*id3tag_init__)(lame_global_flags *);
    typedef void (*id3tag_set_title__)(lame_global_flags *, const char* title);
    typedef void (*id3tag_set_artist__)(lame_global_flags *, const char* artist);
    typedef void (*id3tag_set_album__)(lame_global_flags *, const char* album);
    // Since lame 3.98
    // force addition of version 2 tag
    typedef void (*id3tag_add_v2__)   (lame_t gfp);

    id3tag_init__                       id3tag_init;
    id3tag_set_title__                  id3tag_set_title;
    id3tag_set_artist__                 id3tag_set_artist;
    id3tag_set_album__                  id3tag_set_album;
    id3tag_add_v2__                     id3tag_add_v2;

    QString m_metaDataTitle;
    QString m_metaDataArtist;
    QString m_metaDataAlbum;

    int m_bitrate;
    int m_vbr_index;
    vbr_mode m_encoding_mode;
    MPEG_mode_e m_stereo_mode;
    unsigned char *m_bufferOut;
    int m_bufferOutSize;
    float *m_bufferIn[2];
    int m_bufferInSize;

    EncoderCallback* m_pCallback;
    TrackPointer m_pMetaData;
    QLibrary* m_library;
    QFile m_mp3file;
};

#endif
