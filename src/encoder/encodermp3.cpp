/****************************************************************************
                   encodermp3.cpp  - mp3 encoder for mixxx
                             -------------------
    copyright            : (C) 2007 by Wesley Stessens
                           (C) 2009 by Phillip Whelan (rewritten for mp3)
                           (C) 2010 by Tobias Rafreider (fixes for broadcast, dynamic loading of lame_enc.dll, etc)

    Libmp3lame API:
    http://lame.cvs.sourceforge.net/viewvc/lame/lame/API?view=markup
    http://lame.cvs.sourceforge.net/viewvc/lame/lame/include/lame.h?view=markup

    Older BladeEncDll API:
    http://lame.cvs.sourceforge.net/viewvc/lame/lame/Dll/BladeMP3EncDLL.h?view=markup

*****************************************************************************/

#include <QtDebug>
#include <QObject>
#include <limits.h>

#include "encoder/encodermp3.h"
#include "encoder/encodermp3settings.h"
#include "encoder/encodercallback.h"
#include "errordialoghandler.h"

// Automatic thresholds for switching the encoder to mono
// They have been choosen by testing and to keep the same number
// of values for the slider.
// The threshold of bitrate (CBR/ABR) at which the encoder
// with switch to mono encoding
const int EncoderMp3::MONO_BITRATE_TRESHOLD = 100;
// The threshold of quality (VBR) at which the encoder
// with switch to mono encoding. Values from 0 to 6 encode at 44Khz
const int EncoderMp3::MONO_VBR_THRESHOLD = 8;
// Quality offset to substract to the quality value when
// switching to mono encoding.
const int EncoderMp3::MONO_VBR_OFFSET = 4;


EncoderMp3::EncoderMp3(EncoderCallback* pCallback)
  : m_lameFlags(nullptr),
    m_bitrate(128),
    m_bufferOut(nullptr),
    m_bufferOutSize(0),
    /*
     * @ Author: Tobias Rafreider
     * Nobody has initialized the field before my code review.  At runtime the
     * Integer field was inialized by a large random value such that the
     * following pointer fields were never initialized in the methods
     * 'bufferOutGrow()' and 'bufferInGrow()' --> Valgrind shows invalid writes
     * :-)
     *
     * m_bufferOut = (unsigned char *)realloc(m_bufferOut, size);
     * m_bufferIn[0] = (float *)realloc(m_bufferIn[0], size * sizeof(float));
     * m_bufferIn[1] = (float *)realloc(m_bufferIn[1], size * sizeof(float));
     *
     * This has solved many segfaults when using and even closing broadcast
     * along with LAME.  This bug was detected by using Valgrind memory analyzer
     *
     */
    m_bufferInSize(0),
    m_pCallback(pCallback),
    m_library(nullptr) {
    m_bufferIn[0] = nullptr;
    m_bufferIn[1] = nullptr;

    //These are the function pointers for lame
    lame_init = nullptr;
    lame_set_num_channels = nullptr;
    lame_set_in_samplerate = nullptr;
    lame_set_out_samplerate = nullptr;
    lame_close = nullptr;
    lame_set_brate = nullptr;
    lame_set_mode = nullptr;
    lame_set_quality = nullptr;
    lame_set_bWriteVbrTag = nullptr;
    lame_encode_buffer_float = nullptr;
    lame_init_params = nullptr;
    lame_encode_flush = nullptr;
    lame_set_VBR = nullptr;
    lame_set_VBR_q = nullptr;
    lame_set_VBR_quality = nullptr;
    lame_set_VBR_mean_bitrate_kbps = nullptr;
    lame_encode_buffer_interleaved_ieee_float = nullptr;
    lame_get_lametag_frame = nullptr;

    id3tag_init = nullptr;
    id3tag_set_title = nullptr;
    id3tag_set_artist = nullptr;
    id3tag_set_album = nullptr;
    id3tag_add_v2 = nullptr;

    /*
     * Load shared library
     */
    QStringList libnames;
    QString libname = "";
#ifdef __LINUX__
    libnames << "mp3lame";
#elif __WINDOWS__
    libnames << "lame_enc.dll";
    libnames << "libmp3lame.dll";
#elif __APPLE__
    libnames << "/usr/local/lib/libmp3lame.dylib";
    //Using MacPorts (former DarwinPorts) results in ...
    libnames << "/opt/local/lib/libmp3lame.dylib";
#endif

    for (const auto& libname : libnames) {
        m_library = new QLibrary(libname, 0);
        if (m_library->load()) {
            qDebug() << "Successfully loaded encoder library " << libname;
            break;
        } else {
            qWarning() << "Failed to load " << libname << ", " << m_library->errorString();
        }
        delete m_library;
        m_library = nullptr;
    }

    if (!m_library || !m_library->isLoaded()) {
        ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
        props->setType(DLG_WARNING);
        props->setTitle(QObject::tr("Encoder"));
        QString missingCodec = QObject::tr("<html>Mixxx cannot record or stream in MP3 without the MP3 encoder &quot;lame&quot;. Due to licensing issues, we cannot include this with Mixxx. To record or stream in MP3, you must download <b>libmp3lame</b> and install it on your system. <p>See <a href='http://mixxx.org/wiki/doku.php/internet_broadcasting#%1'>Mixxx Wiki</a> for more information. </html>");

#ifdef __LINUX__
        missingCodec = missingCodec.arg("linux");
#elif __WINDOWS__
        missingCodec = missingCodec.arg("windows");
#elif __APPLE__
        missingCodec = missingCodec.arg("mac_osx");
#endif
        props->setText(missingCodec);
        props->setKey(missingCodec);
        ErrorDialogHandler::instance()->requestErrorDialog(props);
        return;
    }

    typedef const char* (*get_lame_version__)(void);
    get_lame_version__ get_lame_version = (get_lame_version__)m_library->resolve("get_lame_version");


    // initialize function pointers
    lame_init                   = (lame_init__)m_library->resolve("lame_init");
    lame_set_num_channels       = (lame_set_num_channels__)m_library->resolve("lame_set_num_channels");
    lame_set_in_samplerate      = (lame_set_in_samplerate__)m_library->resolve("lame_set_in_samplerate");
    lame_set_out_samplerate     = (lame_set_out_samplerate__)m_library->resolve("lame_set_out_samplerate");
    lame_close                  = (lame_close__)m_library->resolve("lame_close");
    lame_set_brate              = (lame_set_brate__)m_library->resolve("lame_set_brate");
    lame_set_mode               = (lame_set_mode__)m_library->resolve("lame_set_mode");
    lame_set_quality            = (lame_set_quality__)m_library->resolve("lame_set_quality");
    lame_set_bWriteVbrTag       = (lame_set_bWriteVbrTag__)m_library->resolve("lame_set_bWriteVbrTag");
    lame_encode_buffer_float    = (lame_encode_buffer_float__)m_library->resolve("lame_encode_buffer_float");
    lame_init_params            = (lame_init_params__)m_library->resolve("lame_init_params");
    lame_encode_flush           = (lame_encode_flush__)m_library->resolve("lame_encode_flush");

    lame_set_VBR                = (lame_set_VBR__)m_library->resolve("lame_set_VBR");
    lame_set_VBR_q              = (lame_set_VBR_q__)m_library->resolve("lame_set_VBR_q");
    lame_set_VBR_quality        = (lame_set_VBR_quality__)m_library->resolve("lame_set_VBR_quality");

    lame_set_VBR_mean_bitrate_kbps =
              (lame_set_VBR_mean_bitrate_kbps__)m_library->resolve("lame_set_VBR_mean_bitrate_kbps");
    lame_encode_buffer_interleaved_ieee_float =
              (lame_encode_buffer_interleaved_ieee_float__)m_library->resolve("lame_encode_buffer_interleaved_ieee_float");
    lame_get_lametag_frame      = (lame_get_lametag_frame__)m_library->resolve("lame_get_lametag_frame");


    id3tag_init                 = (id3tag_init__)m_library->resolve("id3tag_init");
    id3tag_set_title            = (id3tag_set_title__)m_library->resolve("id3tag_set_title");
    id3tag_set_artist           = (id3tag_set_artist__)m_library->resolve("id3tag_set_artist");
    id3tag_set_album            = (id3tag_set_album__)m_library->resolve("id3tag_set_album");

    id3tag_add_v2               = (id3tag_add_v2__)m_library->resolve("id3tag_add_v2");;

    /*
     * Check if all function pointers are not NULL
     * Otherwise, the lame_enc.dll, libmp3lame.so or libmp3lame.mylib do not comply with the official header lame.h
     * Indicates a modified lame version
     *
     * Should not happen on Linux, but many lame binaries for Windows are modified.
     */
    if(!lame_init ||
       !lame_set_num_channels ||
       !lame_set_in_samplerate ||
       !lame_set_out_samplerate ||
       !lame_close ||
       !lame_set_brate ||
       !lame_set_mode ||
       !lame_set_quality ||
       !lame_set_bWriteVbrTag ||
       !lame_encode_buffer_float ||
       !lame_init_params ||
       !lame_encode_flush ||
       !lame_set_VBR ||
       !lame_set_VBR_q ||
       !lame_set_VBR_mean_bitrate_kbps ||
       !lame_get_lametag_frame ||
       !get_lame_version ||
       !id3tag_init ||
       !id3tag_set_title ||
       !id3tag_set_artist ||
       !id3tag_set_album)
    {
        m_library->unload();
        delete m_library;
        m_library = nullptr;
        //print qDebugs to detect which function pointers are null
        qDebug() << "lame_init: " << lame_init;
        qDebug() << "lame_set_num_channels: " << lame_set_num_channels;
        qDebug() << "lame_set_in_samplerate: " << lame_set_in_samplerate;
        qDebug() << "lame_set_out_samplerate: " << lame_set_out_samplerate;
        qDebug() << "lame_close: " << lame_close;
        qDebug() << "lame_set_brate " << lame_set_brate;
        qDebug() << "lame_set_mode: " << lame_set_mode;
        qDebug() << "lame_set_quality: " << lame_set_quality;
        qDebug() << "lame_set_bWriteVbrTag: " << lame_set_bWriteVbrTag;
        qDebug() << "lame_encode_buffer_float: " << lame_encode_buffer_float;
        qDebug() << "lame_init_params: " << lame_init_params;
        qDebug() << "lame_encode_flush: " << lame_encode_flush;
        qDebug() << "lame_set_VBR: " << lame_set_VBR;
        qDebug() << "lame_set_VBR_q: " << lame_set_VBR_q;
        qDebug() << "lame_set_VBR_mean_bitrate_kbps: " << lame_set_VBR_mean_bitrate_kbps;
        qDebug() << "get_lame_version: " << get_lame_version;
        qDebug() << "id3tag_init: " << id3tag_init;
        qDebug() << "id3tag_set_title : " << id3tag_set_title ;
        qDebug() << "id3tag_set_artist: " << id3tag_set_artist;
        qDebug() << "id3tag_set_album  " << id3tag_set_album ;

        ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
        props->setType(DLG_WARNING);
        props->setTitle(QObject::tr("Encoder"));
        QString key = QObject::tr("<html>Mixxx has detected that you use a modified version of libmp3lame. See <a href='http://mixxx.org/wiki/doku.php/internet_broadcasting'>Mixxx Wiki</a> for more information.</html>");
        props->setText(key);
        props->setKey(key);
        ErrorDialogHandler::instance()->requestErrorDialog(props);
        return;
    }
    qDebug() << "Loaded libmp3lame version " << get_lame_version();
    qDebug() << "lame_set_VBR_quality: " << QString((lame_set_VBR_quality == nullptr) ? "missing" : "present")
        << " lame_encode_buffer_interleaved_ieee_float: "
        << QString((lame_encode_buffer_interleaved_ieee_float == nullptr) ? "missing" : "present")
        << " id3tag_add_v2: " << QString((id3tag_add_v2 == nullptr) ? "missing" : "present");
}

// Destructor
EncoderMp3::~EncoderMp3() {
    if (m_library != nullptr && m_library->isLoaded()) {
        flush();
        lame_close(m_lameFlags);
        m_library->unload(); //unload dll, so, ...
        qDebug() << "Unloaded libmp3lame ";
        m_library = nullptr;
    }
    //free requested buffers
    if (m_bufferIn[0] != nullptr)
        delete m_bufferIn[0];
    if (m_bufferIn[1] != nullptr)
        delete m_bufferIn[1];
    if (m_bufferOut != nullptr)
        delete m_bufferOut;
}

void EncoderMp3::setEncoderSettings(const EncoderSettings& settings)
{
    m_bitrate = settings.getQuality();

    int modeoption = settings.getSelectedOption(EncoderMp3Settings::ENCODING_MODE_GROUP);
    m_encoding_mode = (modeoption==0) ? vbr_off : (modeoption==1) ? vbr_abr : vbr_default;

    if (m_encoding_mode == vbr_off) {
        if (m_bitrate > MONO_BITRATE_TRESHOLD ) {
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
    switch(settings.getChannelMode()) {
        case EncoderSettings::ChannelMode::MONO:  m_stereo_mode = MONO; break;
        case EncoderSettings::ChannelMode::STEREO: m_stereo_mode = JOINT_STEREO; break;
        default: break;
    }
}

/*
 * Grow the outBuffer if needed.
 */

int EncoderMp3::bufferOutGrow(int size) {
    if (m_bufferOutSize >= size)
        return 0;

    m_bufferOut = (unsigned char *)realloc(m_bufferOut, size);
    if (m_bufferOut == nullptr)
        return -1;

    m_bufferOutSize = size;
    return 0;
}

/*
 * Grow the inBuffer(s) if needed.
 */

int EncoderMp3::bufferInGrow(int size) {
    if (m_bufferInSize >= size)
        return 0;

    m_bufferIn[0] = (float *)realloc(m_bufferIn[0], size * sizeof(float));
    m_bufferIn[1] = (float *)realloc(m_bufferIn[1], size * sizeof(float));
    if ((m_bufferIn[0] == nullptr) || (m_bufferIn[1] == nullptr))
        return -1;

    m_bufferInSize = size;
    return 0;
}

// Using this method requires to call method 'write()' or 'sendPackages()'
// depending on which context you use the class (broadcast or recording to HDD)
void EncoderMp3::flush() {
    if (m_library == nullptr || !m_library->isLoaded())
        return;
    int rc = 0;
    /**Flush also writes ID3 tags **/
    rc = lame_encode_flush(m_lameFlags, m_bufferOut, m_bufferOutSize);
    if (rc < 0) {
        return;
    }
    // end encoded audio to broadcast or file
    m_pCallback->write(nullptr, m_bufferOut, 0, rc);

    // Write the lame/xing header.
    rc = lame_get_lametag_frame(m_lameFlags, m_bufferOut, m_bufferOutSize);
    if (rc != m_bufferOutSize) {
        bufferOutGrow(rc);
        rc = lame_get_lametag_frame(m_lameFlags, m_bufferOut, m_bufferOutSize);
    }
    m_pCallback->seek(0);
    m_pCallback->write(nullptr, m_bufferOut, 0, rc);
}

void EncoderMp3::encodeBuffer(const CSAMPLE *samples, const int size) {
    if (m_library == nullptr || !m_library->isLoaded())
        return;
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
    return;
}

int EncoderMp3::initEncoder(int samplerate, QString errorMessage) {
    if (m_library == nullptr || !m_library->isLoaded()) {
        errorMessage  = "MP3 recording is not supported. Lame could not be initialized";
        return -1;
    }

    unsigned long samplerate_in = samplerate;
    // samplerate_out 0 means "let LAME pick the appropiate one"
    unsigned long samplerate_out = (samplerate_in > 48000 ? 48000 : 0);

    m_lameFlags = lame_init();

    if (m_lameFlags == nullptr) {
        qDebug() << "Unable to initialize MP3";
        errorMessage  = "MP3 recording is not supported. Lame could not be initialized";
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

    int ret;
    if ((ret = lame_init_params(m_lameFlags)) < 0) {
        qDebug() << "Unable to initialize MP3 parameters. return code:" << ret;
        errorMessage  = "MP3 recording is not supported. Lame could not be initialized.";
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
