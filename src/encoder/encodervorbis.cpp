#include <stdlib.h> // needed for random num gen
#include <time.h> // needed for random num gen
#include <QtDebug>

#include "encoder/encodervorbis.h"
#include "encoder/encodercallback.h"

// Automatic thresholds for switching the encoder to mono
// They have been chosen by testing and to keep the same number
// of values for the slider.
// The threshold of bitrate at which the encoder
// with switch to mono encoding
const int EncoderVorbis::MONO_BITRATE_THRESHOLD = 70;


EncoderVorbis::EncoderVorbis(EncoderCallback* pCallback)
        : m_bStreamInitialized(false),
          m_vblock({}),
          m_vdsp({}),
          m_vinfo({}),
          m_vcomment({}),
          m_header_write(false),
          m_pCallback(pCallback),
          m_bitrate(128),
          m_channels(2) {
}

EncoderVorbis::~EncoderVorbis() {
    if (m_bStreamInitialized) {
        ogg_stream_clear(&m_oggs);
        vorbis_block_clear(&m_vblock);
        vorbis_dsp_clear(&m_vdsp);
        vorbis_comment_clear(&m_vcomment);
        vorbis_info_clear(&m_vinfo);
    }
}

void EncoderVorbis::setEncoderSettings(const EncoderSettings& settings)
{
    m_bitrate = settings.getQuality();
    // Check if the user has forced a stereo mode.
    switch(settings.getChannelMode()) {
        case EncoderSettings::ChannelMode::MONO:  m_channels = 1; break;
        case EncoderSettings::ChannelMode::STEREO: m_channels = 2; break;
        case EncoderSettings::ChannelMode::AUTOMATIC: // fallthrough
        default:
            if (m_bitrate > MONO_BITRATE_THRESHOLD ) {
                m_channels = 2;
            }
            else {
                m_channels = 1;
            }
        break;
    }

}

// call sendPackages() or write() after 'flush()' as outlined in enginebroadcast.cpp
void EncoderVorbis::flush() {
    vorbis_analysis_wrote(&m_vdsp, 0);
    writePage();
}

/*
  Get new random serial number
  -> returns random number
*/
int EncoderVorbis::getSerial()
{
    static int prevSerial = 0;
    int serial = rand();
    while (prevSerial == serial) {
        serial = rand();
    }
    prevSerial = serial;
    qDebug() << "RETURNING SERIAL " << serial;
    return serial;
}

void EncoderVorbis::writePage() {

    /*
     * Vorbis streams begin with three headers; the initial header (with
     * most of the codec setup parameters) which is mandated by the Ogg
     * bitstream spec.  The second header holds any comment fields.  The
     * third header holds the bitstream codebook.  We merely need to
     * make the headers, then pass them to libvorbis one at a time;
     * libvorbis handles the additional Ogg bitstream constraints
         */


    // Write header only once after stream has been initialized
    int result;
    if (m_header_write) {
        while (true) {
            result = ogg_stream_flush(&m_oggs, &m_oggpage);
            if (result == 0) {
                break;
            }
            m_pCallback->write(
                    m_oggpage.header,
                    m_oggpage.body,
                    m_oggpage.header_len,
                    m_oggpage.body_len);
        }
        m_header_write = false;
    }

    while (vorbis_analysis_blockout(&m_vdsp, &m_vblock) == 1) {
        vorbis_analysis(&m_vblock, nullptr);
        vorbis_bitrate_addblock(&m_vblock);
        while (vorbis_bitrate_flushpacket(&m_vdsp, &m_oggpacket)) {
            // weld packet into bitstream
            ogg_stream_packetin(&m_oggs, &m_oggpacket);
            // write out pages
            bool eos = false;
            while (!eos) {
                int result = ogg_stream_pageout(&m_oggs, &m_oggpage);
                if (result == 0) {
                    break;
                }
                m_pCallback->write(m_oggpage.header, m_oggpage.body,
                                   m_oggpage.header_len, m_oggpage.body_len);
                if (ogg_page_eos(&m_oggpage)) {
                    eos = true;
                }
            }
        }
    }
}

void EncoderVorbis::encodeBuffer(const CSAMPLE *samples, const int size) {
    float **buffer = vorbis_analysis_buffer(&m_vdsp, size);

    // Deinterleave samples. We use normalized floats in the engine [-1.0, 1.0]
    // and libvorbis expects samples in the range [-1.0, 1.0] so no conversion
    // is required.
    if (m_channels == 2) {
        for (int i = 0; i < size/2; ++i) {
            buffer[0][i] = samples[i*2];
            buffer[1][i] = samples[i*2+1];
        }
    }
    else {
        for (int i = 0; i < size/2; ++i) {
            buffer[0][i] = (samples[i*2] + samples[i*2+1]) / 2.f;
        }
    }
    /** encodes audio **/
    vorbis_analysis_wrote(&m_vdsp, size/2);
    /** writes the OGG page and sends it to file or stream **/
    writePage();
}

/* Originally called from enginebroadcast.cpp to update metadata information
 * when streaming, however, this causes pops
 *
 * Currently this method is used before init() once to save artist, title and album
*/
void EncoderVorbis::updateMetaData(const QString& artist, const QString& title, const QString& album) {
    m_metaDataTitle = title;
    m_metaDataArtist = artist;
    m_metaDataAlbum = album;
}

void EncoderVorbis::initStream() {
    // set up analysis state and auxiliary encoding storage
    vorbis_analysis_init(&m_vdsp, &m_vinfo);
    vorbis_block_init(&m_vdsp, &m_vblock);

    // set up packet-to-stream encoder; attach a random serial number
    srand(time(nullptr));
    ogg_stream_init(&m_oggs, getSerial());

    // add comment
    vorbis_comment_init(&m_vcomment);
    vorbis_comment_add_tag(&m_vcomment, "ENCODER", "mixxx/libvorbis");
    if (!m_metaDataArtist.isEmpty()) {
        vorbis_comment_add_tag(&m_vcomment, "ARTIST", m_metaDataArtist.toUtf8().constData());
    }
    if (!m_metaDataTitle.isEmpty()) {
        vorbis_comment_add_tag(&m_vcomment, "TITLE", m_metaDataTitle.toUtf8().constData());
    }
    if (!m_metaDataAlbum.isEmpty()) {
        vorbis_comment_add_tag(&m_vcomment, "ALBUM", m_metaDataAlbum.toUtf8().constData());
    }

    // set up the vorbis headers
    ogg_packet headerInit;
    ogg_packet headerComment;
    ogg_packet headerCode;
    vorbis_analysis_headerout(&m_vdsp, &m_vcomment, &headerInit, &headerComment, &headerCode);
    ogg_stream_packetin(&m_oggs, &headerInit);
    ogg_stream_packetin(&m_oggs, &headerComment);
    ogg_stream_packetin(&m_oggs, &headerCode);

    // The encoder is now initialized. The encode method will start streaming by
    // sending the header first.
    m_header_write = true;
    m_bStreamInitialized = true;
}

int EncoderVorbis::initEncoder(int samplerate, QString& errorMessage) {
    vorbis_info_init(&m_vinfo);

    // initialize VBR quality based mode
    int ret = vorbis_encode_init(&m_vinfo, m_channels, samplerate, -1, m_bitrate*1000, -1);

    if (ret == 0) {
        initStream();
    } else {
        qDebug() << "Error initializing OGG recording. IS OGG/Vorbis library installed? Error code: " << ret;
        errorMessage  = "OGG recording is not supported. OGG/Vorbis library could not be initialized.";
        ret = -1;
    };
    return ret;
}
