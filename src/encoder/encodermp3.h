#pragma once

#include <lame/lame.h>

#include "util/types.h"
#include "encoder/encoder.h"

class EncoderMp3 final : public Encoder {
  public:
    static const int MONO_BITRATE_THRESHOLD;
    static const int MONO_VBR_THRESHOLD;
    static const int MONO_VBR_OFFSET;

    EncoderMp3(EncoderCallback* callback=nullptr);
    ~EncoderMp3() override;

    int initEncoder(int samplerate, QString& errorMessage) override;
    void encodeBuffer(const CSAMPLE *samples, const int size) override;
    void updateMetaData(const QString& artist, const QString& title, const QString& album) override;
    void flush() override;
    void setEncoderSettings(const EncoderSettings& settings) override;

  private:
    void initStream();
    int bufferOutGrow(int size);
    int bufferInGrow(int size);

    lame_t m_lameFlags;
    QString m_metaDataTitle;
    QString m_metaDataArtist;
    QString m_metaDataAlbum;

    int m_bitrate;
    int m_vbr_index;
    vbr_mode m_encoding_mode;
    MPEG_mode_e m_stereo_mode;
    unsigned char *m_bufferOut;
    int m_bufferOutSize;
    float* m_bufferIn[2];
    int m_bufferInSize;

    EncoderCallback* m_pCallback;
};
