// encoderopus.h
// Create on August 15th 2017 by Palakis

#ifndef ENCODER_ENCODEROPUS_H
#define ENCODER_ENCODEROPUS_H

#include <QMap>
#include <QString>
#include <QVector>

#include <ogg/ogg.h>
#include <opus/opus.h>

#include "encoder/encoder.h"
#include "encoder/encodercallback.h"
#include "util/fifo.h"
#include "util/memory.h"
#include "util/sample.h"
#include "util/samplebuffer.h"

class EncoderOpus: public Encoder {
  public:
    static int getMasterSamplerate();
    static QString getInvalidSamplerateMessage();

    explicit EncoderOpus(EncoderCallback* pCallback = nullptr);
    ~EncoderOpus() override;

    int initEncoder(int samplerate, QString errorMessage) override;
    void encodeBuffer(const CSAMPLE *samples, const int size) override;
    void updateMetaData(const QString& artist, const QString& title, const QString& album) override;
    void flush() override;
    void setEncoderSettings(const EncoderSettings& settings) override;

  private:
    void initStream();
    void pushHeaderPacket();
    void pushTagsPacket();
    void writePage(ogg_packet* pPacket);
    void processFIFO();

    int m_bitrate;
    int m_bitrateMode;
    int m_channels;
    int m_samplerate;
    int m_readRequired;
    EncoderCallback* m_pCallback;
    FIFO<CSAMPLE> m_fifoBuffer;
    std::unique_ptr<mixxx::SampleBuffer> m_pFifoChunkBuffer;
    OpusEncoder* m_pOpus;
    QVector<unsigned char> m_opusDataBuffer;
    ogg_stream_state m_oggStream;
    ogg_page m_oggPage;
    bool m_header_write;
    int m_packetNumber;
    ogg_int64_t m_granulePos;
    QMap<QString, QString> m_opusComments;
};

#endif // ENCODER_ENCODEROPUS_H
