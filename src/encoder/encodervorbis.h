/***************************************************************************
                     encodervorbis.h  -  vorbis encoder for mixxx
                             -------------------
    copyright            : (C) 2007 by Wesley Stessens
                           (C) 1994 by Xiph.org (encoder example)
                           (C) 1994 Tobias Rafreider (broadcast and recording fixes)
 ***************************************************************************/


#ifndef ENCODERVORBIS_H
#define ENCODERVORBIS_H

// this also includes vorbis/codec.h
#include <vorbis/vorbisenc.h>

#include <QFile>

#include "encoder/encoder.h"
#include "track/track_decl.h"
#include "util/types.h"

class EncoderCallback;

class EncoderVorbis : public Encoder {
  public:
    static const int MONO_BITRATE_THRESHOLD;

    EncoderVorbis(EncoderCallback* pCallback = nullptr);
    ~EncoderVorbis() override;

    int initEncoder(int samplerate, QString errorMessage) override;
    void encodeBuffer(const CSAMPLE *samples, const int size) override;
    void updateMetaData(const QString& artist, const QString& title, const QString& album) override;
    void flush() override;
    void setEncoderSettings(const EncoderSettings& settings) override;

  private:
    int getSerial();
    void initStream();
    bool metaDataHasChanged();
    //Call this method in conjunction with broadcast streaming
    void writePage();

    bool m_bStreamInitialized;
    ogg_stream_state m_oggs;    /* take physical pages, weld into logical stream
                                 of packets */
    ogg_page m_oggpage;         /* Ogg bitstream page: contains Vorbis packets */
    ogg_packet m_oggpacket;     /* raw packet of data */
    vorbis_block m_vblock;      /* local working space for packet-to-PCM */
    vorbis_dsp_state m_vdsp;    /* central working space for packet-to-PCM */
    vorbis_info m_vinfo;        /* stores all static vorbis bitstream settings */
    vorbis_comment m_vcomment;  /* stores all user comments */
    bool m_header_write;

    EncoderCallback* m_pCallback;
    TrackPointer m_pMetaData;
    QString m_metaDataTitle;
    QString m_metaDataArtist;
    QString m_metaDataAlbum;
    int m_bitrate;
    int m_channels;
    QFile m_file;
};

#endif
