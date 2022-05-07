#include <QtDebug>
#include <QObject>
#include <limits.h>

#include "encoder/encodermp3.h"
#include "encoder/encodermp3settings.h"
#include "encoder/encodercallback.h"

// Automatic thresholds for switching the encoder to mono
// They have been chosen by testing and to keep the same number
// of values for the slider.
// The threshold of bitrate (CBR/ABR) at which the encoder
// with switch to mono encoding
const int EncoderMp3::MONO_BITRATE_THRESHOLD = 100;
// The threshold of quality (VBR) at which the encoder
// with switch to mono encoding. Values from 0 to 6 encode at 44Khz
const int EncoderMp3::MONO_VBR_THRESHOLD = 8;
// Quality offset to subtract to the quality value when
// switching to mono encoding.
const int EncoderMp3::MONO_VBR_OFFSET = 4;


EncoderMp3::EncoderMp3(EncoderCallback* pCallback)
        : m_lameFlags(nullptr),
          m_bitrate(128),
          m_bufferOut(nullptr),
          m_bufferOutSize(0),
          m_bufferIn{nullptr, nullptr},
          m_bufferInSize(0),
          m_pCallback(pCallback) {
}

EncoderMp3::~EncoderMp3() {
    flush();
    if (m_lameFlags != nullptr) {
        lame_close(m_lameFlags);
    }
    // free requested buffers
    if (m_bufferIn[0] != nullptr) {
        free(m_bufferIn[0]);
    }
    if (m_bufferIn[1] != nullptr) {
        free(m_bufferIn[1]);
    }
    if (m_bufferOut != nullptr) {
        free(m_bufferOut);
    }
}

void EncoderMp3::setEncoderSettings(const EncoderSettings& settings) {
    m_bitrate = settings.getQuality();

    int modeoption = settings.getSelectedOption(EncoderMp3Settings::ENCODING_MODE_GROUP);
    m_encoding_mode = (modeoption==0) ? vbr_off : (modeoption==1) ? vbr_abr : vbr_default;

    if (m_encoding_mode == vbr_off) {
        if (m_bitrate > MONO_BITRATE_THRESHOLD ) {
            m_stereo_mode = JOINT_STEREO;
        } else {
            m_stereo_mode = MONO;
        }
    } else {
        // Inverting range: vbr 0 best, 9 worst. slider 0 min to max.
        int val = settings.getQualityValues().size() - 1 - settings.getQualityIndex();
        if (val < MONO_VBR_THRESHOLD) {
            m_stereo_mode = JOINT_STEREO;
            m_vbr_index = val;
        } else {
            m_vbr_index = val-4;
            m_stereo_mode = MONO;
        }
    }
    // Check if the user has forced a stereo mode.
    switch (settings.getChannelMode()) {
        case EncoderSettings::ChannelMode::MONO:  m_stereo_mode = MONO; break;
        case EncoderSettings::ChannelMode::STEREO: m_stereo_mode = JOINT_STEREO; break;
        default: break;
    }
}

int EncoderMp3::bufferOutGrow(int size) {
    if (m_bufferOutSize >= size) {
        return 0;
    }

    m_bufferOut = (unsigned char *)realloc(m_bufferOut, size);
    if (m_bufferOut == nullptr) {
        return -1;
    }

    m_bufferOutSize = size;
    return 0;
}

int EncoderMp3::bufferInGrow(int size) {
    if (m_bufferInSize >= size) {
        return 0;
    }

    m_bufferIn[0] = (float *)realloc(m_bufferIn[0], size * sizeof(float));
    m_bufferIn[1] = (float *)realloc(m_bufferIn[1], size * sizeof(float));
    if ((m_bufferIn[0] == nullptr) || (m_bufferIn[1] == nullptr)) {
        return -1;
    }

    m_bufferInSize = size;
    return 0;
}

// Using this method requires to call method 'write()' or 'sendPackages()'
// depending on which context you use the class (broadcast or recording to HDD)
void EncoderMp3::flush() {
    if (m_lameFlags == nullptr) {
        return;
    }
    // Flush also writes ID3 tags.
    int rc = lame_encode_flush(m_lameFlags, m_bufferOut, m_bufferOutSize);
    if (rc < 0) {
        return;
    }
    // end encoded audio to broadcast or file
    m_pCallback->write(nullptr, m_bufferOut, 0, rc);

    // `lame_get_lametag_frame` returns the number of bytes copied into buffer,
    // or the required buffer size, if the provided buffer is too small.
    // Function failed, if the return value is larger than `m_bufferOutSize`!
    int numBytes = static_cast<int>(
            lame_get_lametag_frame(m_lameFlags, m_bufferOut, m_bufferOutSize));
    if (numBytes > m_bufferOutSize) {
        bufferOutGrow(numBytes);
        numBytes = static_cast<int>(lame_get_lametag_frame(
                m_lameFlags, m_bufferOut, m_bufferOutSize));
    }
    // Write the lame/xing header.
    m_pCallback->seek(0);
    m_pCallback->write(nullptr, m_bufferOut, 0, numBytes);
}

void EncoderMp3::encodeBuffer(const CSAMPLE *samples, const int size) {
    if (m_lameFlags == nullptr) {
        return;
    }
    int outsize = 0;
    int rc = 0;

    outsize = (int)((1.25 * size + 7200) + 1);
    bufferOutGrow(outsize);

    bufferInGrow(size);

    // Deinterleave samples. We use normalized floats in the engine [-1.0, 1.0]
    // but LAME expects samples in the range [SHRT_MIN, SHRT_MAX].
    for (int i = 0; i < size/2; ++i) {
        m_bufferIn[0][i] = samples[i*2] * SHRT_MAX;
        m_bufferIn[1][i] = samples[i*2+1] * SHRT_MAX;
    }

    rc = lame_encode_buffer_float(m_lameFlags, m_bufferIn[0], m_bufferIn[1],
                                  size/2, m_bufferOut, m_bufferOutSize);
    if (rc < 0) {
        return;
    }
    //write encoded audio to broadcast stream or file
    m_pCallback->write(nullptr, m_bufferOut, 0, rc);
}

void EncoderMp3::initStream() {
    m_bufferOutSize = (int)((1.25 * 20000 + 7200) + 1);
    m_bufferOut = (unsigned char *)malloc(m_bufferOutSize);

    m_bufferIn[0] = (float *)malloc(m_bufferOutSize * sizeof(float));
    m_bufferIn[1] = (float *)malloc(m_bufferOutSize * sizeof(float));
}

int EncoderMp3::initEncoder(mixxx::audio::SampleRate sampleRate, QString* pUserErrorMessage) {
    unsigned long samplerate_in = sampleRate;
    // samplerate_out 0 means "let LAME pick the appropriate one"
    unsigned long samplerate_out = (samplerate_in > 48000 ? 48000 : 0);

    m_lameFlags = lame_init();

    if (m_lameFlags == nullptr) {
        qDebug() << "Unable to initialize lame";
        if (pUserErrorMessage) {
            *pUserErrorMessage = QObject::tr(
                    "MP3 encoding is not supported. Lame could not be "
                    "initialized");
        }
        return -1;
    }

    lame_set_in_samplerate(m_lameFlags, samplerate_in);
    lame_set_out_samplerate(m_lameFlags, samplerate_out);

    // Input channels into the encoder
    lame_set_num_channels(m_lameFlags, 2);
    // Output channels (on the mp3 file)
    // mode = 0,1,2,3 = stereo, jstereo, dual channel (not supported), mono
    // Note: JOINT_STEREO is not "forced joint stereo" (That is lame_set_force_ms )
    lame_set_mode(m_lameFlags, m_stereo_mode);

    if (m_encoding_mode == vbr_off) {
        qDebug() << " CBR mode with bitrate: " << m_bitrate;
        lame_set_brate(m_lameFlags, m_bitrate);
    } else if (m_encoding_mode == vbr_abr) {
        qDebug() << " ABR mode with bitrate: " << m_bitrate;
        lame_set_VBR(m_lameFlags, vbr_abr);
        lame_set_VBR_mean_bitrate_kbps(m_lameFlags, m_bitrate);
    } else {
        qDebug() << " VBR mode with value: " << m_vbr_index;
        lame_set_VBR(m_lameFlags, vbr_default);
        lame_set_VBR_q(m_lameFlags, m_vbr_index);
    }

    lame_set_quality(m_lameFlags, 2);

    //ID3 Tag if fields are not NULL
    id3tag_init(m_lameFlags);
    if (!m_metaDataTitle.isEmpty()) {
        id3tag_set_title(m_lameFlags, m_metaDataTitle.toLatin1().constData());
    }
    if (!m_metaDataArtist.isEmpty()) {
        id3tag_set_artist(m_lameFlags, m_metaDataArtist.toLatin1().constData());
    }
    if (!m_metaDataAlbum.isEmpty()) {
        id3tag_set_album(m_lameFlags,m_metaDataAlbum.toLatin1().constData());
    }

    int ret = lame_init_params(m_lameFlags);
    if (ret < 0) {
        qDebug() << "Unable to initialize MP3 parameters. return code:" << ret;
        return -1;
    }

    initStream();

    return 0;
}

void EncoderMp3::updateMetaData(const QString& artist, const QString& title, const QString& album) {
    m_metaDataTitle = title;
    m_metaDataArtist = artist;
    m_metaDataAlbum = album;
}
