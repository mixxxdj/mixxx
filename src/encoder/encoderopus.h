#pragma once

#include <ogg/ogg.h>
#include <opus/opus.h>

#include <QMap>
#include <QString>
#include <QVector>
#include <memory>

#include "audio/types.h"
#include "encoder/encoder.h"
#include "util/fifo.h"

class EncoderCallback;
class EncoderSettings;
namespace mixxx {
class SampleBuffer;
} // namespace mixxx

class EncoderOpus: public Encoder {
  public:
    static mixxx::audio::SampleRate getMainSampleRate();
    static QString getInvalidSamplerateMessage();

    explicit EncoderOpus(EncoderCallback* pCallback = nullptr);
    ~EncoderOpus() override;

    int initEncoder(mixxx::audio::SampleRate sampleRate,
            int channels,
            QString* pUserErrorMessage) override;
    void encodeBuffer(const CSAMPLE* samples, const std::size_t bufferSize) override;
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
    mixxx::audio::SampleRate m_sampleRate;
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
