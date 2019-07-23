// encoderopus.cpp
// Create on August 15th 2017 by Palakis

#include <stdlib.h>
#include <QByteArray>
#include <QMapIterator>
#include <QtGlobal>

#include "encoder/encoderopussettings.h"
#include "engine/sidechain/enginesidechain.h"
#include "util/logger.h"

#include "encoder/encoderopus.h"

namespace {
// From libjitsi's Opus encoder:
// 1 byte TOC + maximum frame size (1275)
// See https://tools.ietf.org/html/rfc6716#section-3.2
constexpr int kMaxOpusBufferSize = 1+1275;
// Opus frame duration in milliseconds. Fixed to 60ms
constexpr int kOpusFrameMs = 60;
constexpr int kOpusChannelCount = 2;
// Opus only supports 48 and 96 kHz samplerates
constexpr int kMasterSamplerate = 48000;

const mixxx::Logger kLogger("EncoderOpus");

QString opusErrorString(int error) {
    QString errorString = "";
    switch (error) {
        case OPUS_OK:
            errorString = "OPUS_OK";
            break;
        case OPUS_BAD_ARG:
            errorString = "OPUS_BAD_ARG";
            break;
        case OPUS_BUFFER_TOO_SMALL:
            errorString = "OPUS_BUFFER_TOO_SMALL";
            break;
        case OPUS_INTERNAL_ERROR:
            errorString = "OPUS_INTERNAL_ERROR";
            break;
        case OPUS_INVALID_PACKET:
            errorString = "OPUS_INVALID_PACKET";
            break;
        case OPUS_UNIMPLEMENTED:
            errorString = "OPUS_UNIMPLEMENTED";
            break;
        case OPUS_INVALID_STATE:
            errorString = "OPUS_INVALID_STATE";
            break;
        case OPUS_ALLOC_FAIL:
            errorString = "OPUS_ALLOC_FAIL";
            break;
        default:
            return "Unknown error";
    }
    return errorString + (QString(" (%1)").arg(error));
}

int getSerial() {
    static int prevSerial = 0;

    int serial;
    do {
        serial = qrand();
    } while(prevSerial == serial);

    prevSerial = serial;
    kLogger.debug() << "RETURNING SERIAL " << serial;
    return serial;
}
}

//static
int EncoderOpus::getMasterSamplerate() {
    return kMasterSamplerate;
}

//static
QString EncoderOpus::getInvalidSamplerateMessage() {
    return QObject::tr(
            "Using Opus at samplerates other than 48 kHz "
            "is not supported by the Opus encoder. Please use "
            "48000 Hz in \"Sound Hardware\" preferences "
            "or switch to a different encoding.");
};

EncoderOpus::EncoderOpus(EncoderCallback* pCallback)
    : m_bitrate(0),
      m_bitrateMode(0),
      m_channels(0),
      m_samplerate(0),
      m_readRequired(0),
      m_pCallback(pCallback),
      m_fifoBuffer(EngineSideChain::SIDECHAIN_BUFFER_SIZE * kOpusChannelCount),
      m_pFifoChunkBuffer(),
      m_pOpus(nullptr),
      m_opusDataBuffer(kMaxOpusBufferSize),
      m_header_write(false),
      m_packetNumber(0),
      m_granulePos(0)
{
    // Regarding m_pFifoBuffer:
    // Size the input FIFO buffer with twice the maximum possible sample count that can be
    // processed at once, to avoid skipping frames or waiting for the required sample count
    // and encode at a regular pace.
    // This is set to the buffer size of the sidechain engine because
    // Recording (which uses this engine) sends more samples at once to the encoder than
    // the Live Broadcasting implementation

    m_opusComments.insert("ENCODER", "mixxx/libopus");
    ogg_stream_init(&m_oggStream, qrand());
}

EncoderOpus::~EncoderOpus() {
    if (m_pOpus) {
        opus_encoder_destroy(m_pOpus);
    }

    ogg_stream_clear(&m_oggStream);
}

void EncoderOpus::setEncoderSettings(const EncoderSettings& settings) {
    m_bitrate = settings.getQuality();
    m_bitrateMode = settings.getSelectedOption(EncoderOpusSettings::BITRATE_MODE_GROUP);
    switch (settings.getChannelMode()) {
        case EncoderSettings::ChannelMode::MONO:
            m_channels = 1;
            break;
        case EncoderSettings::ChannelMode::STEREO:
            m_channels = 2;
            break;
        case EncoderSettings::ChannelMode::AUTOMATIC:
            m_channels = 2;
            break;
    }
}

int EncoderOpus::initEncoder(int samplerate, QString errorMessage) {
    Q_UNUSED(errorMessage);

    if (samplerate != kMasterSamplerate) {
        kLogger.warning() << "initEncoder failed: samplerate not supported by Opus";

        const QString invalidSamplerateMessage = getInvalidSamplerateMessage();

        ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
        props->setType(DLG_WARNING);
        props->setTitle(QObject::tr("Encoder"));
        props->setText(invalidSamplerateMessage);
        props->setKey(invalidSamplerateMessage);
        ErrorDialogHandler::instance()->requestErrorDialog(props);
        return -1;
    }
    m_samplerate = samplerate;

    int createResult = 0;
    m_pOpus = opus_encoder_create(m_samplerate, m_channels, OPUS_APPLICATION_AUDIO, &createResult);

    if (createResult != OPUS_OK) {
        kLogger.warning() << "opus_encoder_create failed:" << opusErrorString(createResult);
        return -1;
    }

    // Optimize encoding for high-quality music
    opus_encoder_ctl(m_pOpus, OPUS_SET_COMPLEXITY(10)); // Highest setting
    opus_encoder_ctl(m_pOpus, OPUS_SET_SIGNAL(OPUS_SIGNAL_MUSIC));

    if(m_bitrateMode == OPUS_BITRATE_CONSTRAINED_VBR) {
        // == Constrained VBR ==
        // Default mode, gives the best quality/bitrate compromise
        opus_encoder_ctl(m_pOpus, OPUS_SET_BITRATE(m_bitrate * 1000)); // convert to bits/second
        opus_encoder_ctl(m_pOpus, OPUS_SET_VBR(1)); // default value in libopus
        opus_encoder_ctl(m_pOpus, OPUS_SET_VBR_CONSTRAINT(1)); // Constrained VBR
    } else if(m_bitrateMode == OPUS_BITRATE_CBR) {
        // == CBR (quality loss at low bitrates) ==
        opus_encoder_ctl(m_pOpus, OPUS_SET_BITRATE(m_bitrate * 1000)); // convert to bits/second
        opus_encoder_ctl(m_pOpus, OPUS_SET_VBR(0));
    } else if(m_bitrateMode == OPUS_BITRATE_VBR) {
        // == Full VBR ==
        opus_encoder_ctl(m_pOpus, OPUS_SET_VBR(1));
        opus_encoder_ctl(m_pOpus, OPUS_SET_VBR_CONSTRAINT(0)); // Unconstrained VBR
    }

    double samplingPeriodMs = ( 1.0 / ((double)m_samplerate) ) * 1000.0;
    double samplesPerChannel = kOpusFrameMs / samplingPeriodMs;

    m_readRequired = samplesPerChannel * m_channels;
    m_pFifoChunkBuffer = std::make_unique<mixxx::SampleBuffer>(m_readRequired);
    initStream();

    return 0;
}

void EncoderOpus::initStream() {
    ogg_stream_clear(&m_oggStream);
    ogg_stream_init(&m_oggStream, getSerial());
    m_header_write = true;
    m_granulePos = 0;
    m_packetNumber = 0;

    pushHeaderPacket();
    pushTagsPacket();
}

// Binary header construction is done manually to properly
// handle endianness of multi-byte number fields
void EncoderOpus::pushHeaderPacket() {
    // Opus identification header
    // Format from https://tools.ietf.org/html/rfc7845.html#section-5.1

    // Header buffer size:
    // - Magic signature: 8 bytes
    // - Version: 1 byte
    // - Channel count: 1 byte
    // - Pre-skip: 2 bytes
    // - Samplerate: 4 bytes
    // - Output Gain: 2 bytes
    // - Mapping family: 1 byte
    // - Channel mapping table: ignored
    // Total: 19 bytes
    const int frameSize = 19;
    QByteArray frame;

    // Magic signature (8 bytes)
    frame.append("OpusHead", 8);

    // Version number (1 byte, fixed to 1)
    frame.append(0x01);

    // Channel count (1 byte)
    frame.append((unsigned char)m_channels);

    // Pre-skip (2 bytes, little-endian)
    int preskip = 0;
    opus_encoder_ctl(m_pOpus, OPUS_GET_LOOKAHEAD(&preskip));
    for (int x = 0; x < 2; x++) {
        unsigned char preskipByte = (preskip >> (x*8)) & 0xFF;
        frame.append(preskipByte);
    }

    // Sample rate (4 bytes, little endian)
    for (int x = 0; x < 4; x++) {
        unsigned char samplerateByte = (m_samplerate >> (x*8)) & 0xFF;
        frame.append(samplerateByte);
    }

    // Output gain (2 bytes, little-endian, fixed to 0)
    frame.append((char)0x00);
    frame.append((char)0x00);

    // Channel mapping (1 byte, fixed to 0, means one stream)
    frame.append((char)0x00);

    // Ignore channel mapping table

    // Assert the built frame is of correct size
    int actualFrameSize = frame.size();
    if (actualFrameSize != frameSize) {
        kLogger.warning() <<
                QString("pushHeaderPacket: wrong frame size! expected: %1 - actual: %2")
                .arg(frameSize).arg(actualFrameSize);
    }

    // Push finished header to stream
    ogg_packet packet;
    packet.b_o_s = 1;
    packet.e_o_s = 0;
    packet.granulepos = 0;
    packet.packetno = m_packetNumber++;
    packet.packet = reinterpret_cast<unsigned char*>(frame.data());
    packet.bytes = frameSize;

    if (ogg_stream_packetin(&m_oggStream, &packet) != 0) {
        // return value != 0 means an internal error happened
        kLogger.warning() <<
                "pushHeaderPacket: failed to send packet to Ogg stream";
    }
}

void EncoderOpus::pushTagsPacket() {
    // Opus comment header
    // Format from https://tools.ietf.org/html/rfc7845.html#section-5.2

    QByteArray combinedComments;
    int commentCount = 0;

    const char* vendorString = opus_get_version_string();
    int vendorStringLength = strlen(vendorString);

    // == Compute tags frame size ==
    // - Magic signature: 8 bytes
    // - Vendor string length: 4 bytes
    // - Vendor string: dynamic size
    // - Comment list length: 4 bytes
    int frameSize = 8 + 4 + vendorStringLength + 4;
    // - Comment list: dynamic size
    QMapIterator<QString, QString> iter(m_opusComments);
    while(iter.hasNext()) {
        iter.next();
        QString comment = iter.key() + "=" + iter.value();
        QByteArray commentBytes = comment.toUtf8();
        int commentBytesLength = commentBytes.size();

        // One comment is:
        // - 4 bytes of string length
        // - string data

        // Add comment length field and data to comments "list"
        for (int x = 0; x < 4; x++) {
            unsigned char fieldValue = (commentBytesLength >> (x*8)) & 0xFF;
            combinedComments.append(fieldValue);
        }
        combinedComments.append(commentBytes);

        // Don't forget to include this comment in the overall size calculation
        frameSize += (4 + commentBytesLength);
        commentCount++;
    }

    // == Actual frame building ==
    QByteArray frame;

    // Magic signature (8 bytes)
    frame.append("OpusTags", 8);

    // Vendor string (mandatory)
    // length field (4 bytes, little-endian) + actual string
    // Write length field
    for (int x = 0; x < 4; x++) {
        unsigned char lengthByte = (vendorStringLength >> (x*8)) & 0xFF;
        frame.append(lengthByte);
    }
    // Write string
    frame.append(vendorString, vendorStringLength);

    // Number of comments (4 bytes, little-endian)
    for (int x = 0; x < 4; x++) {
        unsigned char commentCountByte = (commentCount >> (x*8)) & 0xFF;
        frame.append(commentCountByte);
    }

    // Comment list (dynamic size)
    int commentListLength = combinedComments.size();
    frame.append(combinedComments.constData(), commentListLength);

    // Assert the built frame is of correct size
    int actualFrameSize = frame.size();
    if (actualFrameSize != frameSize) {
        kLogger.warning() <<
                QString("pushTagsPacket: wrong frame size! expected: %1 - actual: %2")
                .arg(frameSize).arg(actualFrameSize);
    }

    // Push finished tags frame to stream
    ogg_packet packet;
    packet.b_o_s = 0;
    packet.e_o_s = 0;
    packet.granulepos = 0;
    packet.packetno = m_packetNumber++;
    packet.packet = reinterpret_cast<unsigned char*>(frame.data());
    packet.bytes = frameSize;

    if (ogg_stream_packetin(&m_oggStream, &packet) != 0) {
        // return value != 0 means an internal error happened
        kLogger.warning() <<
                "pushTagsPacket: failed to send packet to Ogg stream";
    }
}

void EncoderOpus::encodeBuffer(const CSAMPLE *samples, const int size) {
    if (!m_pOpus) {
        return;
    }

    int writeRequired = size;
    int writeAvailable = m_fifoBuffer.writeAvailable();
    if (writeRequired > writeAvailable) {
        kLogger.warning() << "FIFO buffer too small, losing samples!"
                          << "required:" << writeRequired
                          << "; available: " << writeAvailable;
    }

    int writeCount = math_min(writeRequired, writeAvailable);
    if (writeCount > 0) {
        m_fifoBuffer.write(samples, writeCount);
    }

    processFIFO();
}

void EncoderOpus::processFIFO() {
    while (m_fifoBuffer.readAvailable() >= m_readRequired) {
        m_fifoBuffer.read(m_pFifoChunkBuffer->data(), m_readRequired);

        if ((m_readRequired % m_channels) != 0) {
            kLogger.warning() << "processFIFO: channel count doesn't match chunk size";
        }

        int samplesPerChannel = m_readRequired / m_channels;
        int result = opus_encode_float(m_pOpus,
                m_pFifoChunkBuffer->data(), samplesPerChannel,
                m_opusDataBuffer.data(), kMaxOpusBufferSize);

        if (result < 1) {
            kLogger.warning() << "opus_encode_float failed:" << opusErrorString(result);
            return;
        }

        ogg_packet packet;
        packet.b_o_s = 0;
        packet.e_o_s = 0;
        packet.granulepos = m_granulePos;
        packet.packetno = m_packetNumber;
        packet.packet = m_opusDataBuffer.data();
        packet.bytes = result;

        m_granulePos += samplesPerChannel;
        m_packetNumber += 1;

        writePage(&packet);
    }
}

void EncoderOpus::writePage(ogg_packet* pPacket) {
    if (!pPacket) {
        return;
    }

    // Push headers prepared by initStream if not already done
    if (m_header_write) {
        while (true) {
            int result = ogg_stream_flush(&m_oggStream, &m_oggPage);
            if (result == 0)
                break;

            kLogger.debug() << "pushing headers to output";
            m_pCallback->write(m_oggPage.header, m_oggPage.body,
                               m_oggPage.header_len, m_oggPage.body_len);
        }
        m_header_write = false;
    }

    // Push Opus Ogg packets to the stream
    if (ogg_stream_packetin(&m_oggStream, pPacket) != 0) {
        // return value != 0 means an internal error happened
        kLogger.warning() <<
                "writePage: failed to send packet to Ogg stream";
    }

    // Try to send available Ogg pages to the output
    do {
        if (ogg_stream_pageout(&m_oggStream, &m_oggPage) == 0) {
            break;
        }

        m_pCallback->write(m_oggPage.header, m_oggPage.body,
                           m_oggPage.header_len, m_oggPage.body_len);
    } while(!ogg_page_eos(&m_oggPage));
}

void EncoderOpus::updateMetaData(const QString& artist, const QString& title, const QString& album) {
    m_opusComments.insert("ARTIST", artist);
    m_opusComments.insert("TITLE", title);
    m_opusComments.insert("ALBUM", album);
}

void EncoderOpus::flush() {
    // At this point there may still be samples in the FIFO buffer
    processFIFO();
}
