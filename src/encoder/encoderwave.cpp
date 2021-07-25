#include <QtDebug>

#include "encoder/encoderwave.h"

#include "encoder/encodercallback.h"
#include "errordialoghandler.h"
#include "recording/defs_recording.h"


// The virtual file context must return the length of the virtual file in bytes.
static sf_count_t  sf_f_get_filelen (void *user_data)
{
    EncoderCallback* pCallback = static_cast<EncoderCallback*>(user_data);
    return pCallback->filelen();
}
// The virtual file context must seek to offset using the seek mode provided
// by whence which is one of
//      SEEK_CUR
//      SEEK_SET
//      SEEK_END
// The return value must contain the new offset in the file.
static sf_count_t  sf_f_seek (sf_count_t offset, int whence, void *user_data)
{
    sf_count_t new_offset;
    EncoderCallback* pCallback = static_cast<EncoderCallback*>(user_data);
    if (whence == SEEK_SET) {
        new_offset = offset;
        pCallback->seek(static_cast<int>(new_offset));
    } else if (whence == SEEK_CUR) {
        new_offset = pCallback->tell()+offset;
        pCallback->seek(static_cast<int>(new_offset));
    } else {
        new_offset =  pCallback->filelen()-offset;
        pCallback->seek(static_cast<int>(new_offset));
    }
    return new_offset;
}
// The virtual file context must copy ("read") "count" bytes into the buffer
// provided by ptr and return the count of actually copied bytes.
static sf_count_t  sf_f_read (void *ptr, sf_count_t count, void *user_data)
{
    Q_UNUSED(ptr);
    Q_UNUSED(count);
    Q_UNUSED(user_data);
    qWarning() << "sf_f_read called for EncoderWave. Call not implemented!";
    return 0;
}

// The virtual file context must process "count" bytes stored in the buffer passed
//  with ptr and return the count of actually processed bytes.
static sf_count_t sf_f_write (const void *ptr, sf_count_t count, void *user_data)
{
    EncoderCallback* pCallback = static_cast<EncoderCallback*>(user_data);
    pCallback->write(nullptr, static_cast<const unsigned char*>(ptr), 0, static_cast<int>(count));
    return count;
}

// Return the current position of the virtual file context.
static sf_count_t  sf_f_tell (void *user_data)
{
    EncoderCallback* pCallback = static_cast<EncoderCallback*>(user_data);
    return pCallback->tell();
}




EncoderWave::EncoderWave(EncoderCallback* pCallback)
        : m_pCallback(pCallback),
          m_pSndfile(nullptr) {
    m_sfInfo.frames = 0;
    m_sfInfo.samplerate = 0;
    m_sfInfo.channels = 0;
    m_sfInfo.format = 0;
    m_sfInfo.sections = 0;
    m_sfInfo.seekable = 0;

    // Libsndfile calls the callbacks provided by the SF_VIRTUAL_IO structure
    // when opening, reading and writing to the virtual file context. The user_data
    // pointer is a user defined context which will be available in the callbacks.
    m_virtualIo.get_filelen = sf_f_get_filelen;
    m_virtualIo.seek = sf_f_seek;
    m_virtualIo.read = sf_f_read;
    m_virtualIo.write = sf_f_write;
    m_virtualIo.tell = sf_f_tell;
}

EncoderWave::~EncoderWave() {
    if (m_pSndfile != nullptr) {
        sf_close(m_pSndfile);
    }
}

void EncoderWave::setEncoderSettings(const EncoderSettings& settings) {
    const EncoderWaveSettings& wavesettings = reinterpret_cast<const EncoderWaveSettings&>(settings);
    QString format = wavesettings.getFormat();
    if (format == ENCODING_WAVE) {
        m_sfInfo.format = SF_FORMAT_WAV;
    } else if (format == ENCODING_AIFF) {
        m_sfInfo.format = SF_FORMAT_AIFF;
    } else {
        qWarning() << "Unexpected Format when setting EncoderWave: " << format << ". Reverting to wav";
        // Other possibly interesting formats
        // There is a n option for RF64 to automatically downgrade to RIFF WAV if less than 4GB using an
        // sf_command, so it could be interesting to use it in place of FORMAT_WAVE.
        // SF_FORMAT_W64          = 0x0B0000,     /* Sonic Foundry's 64 bit RIFF/WAV */
        // SF_FORMAT_RF64         = 0x220000,     /* RF64 WAV file */

        // I guess this one is WAVEFORMATEXTENSIBLE, not WAVEFORMATEX.
        // Not really useful for us since it's mostly for multichannel setups.
        // SF_FORMAT_WAVEX        = 0x130000,     /* MS WAVE with WAVEFORMATEX */

        // SF_FORMAT_CAF          = 0x180000,     /* Core Audio File format */
    }
    int radio = settings.getSelectedOption(EncoderWaveSettings::BITS_GROUP);
    switch(radio) {
        case 0:
            m_sfInfo.format |= SF_FORMAT_PCM_16;
            break;
        case 1:
            m_sfInfo.format |= SF_FORMAT_PCM_24;
            break;
        case 2:
            m_sfInfo.format |= SF_FORMAT_FLOAT;
            break;
        default:
            m_sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
            qWarning() << " Unexpected radio index on EncoderWave: "
                    << radio << ". reverting to PCM 16bits";
            break;
    }
}

// call sendPackages() or write() after 'flush()' as outlined in enginebroadcast.cpp
void EncoderWave::flush() {
    sf_write_sync(m_pSndfile);
}


void EncoderWave::encodeBuffer(const CSAMPLE *pBuffer, const int iBufferSize) {
    sf_write_float(m_pSndfile, pBuffer, iBufferSize);
}

/* Originally called from enginebroadcast.cpp to update metadata information
 * when streaming, however, this causes pops
 *
 * Currently this method is used before init() once to save artist, title and album
*/
void EncoderWave::updateMetaData(const QString& artist, const QString& title, const QString& album) {
    m_metaDataTitle = title;
    m_metaDataArtist = artist;
    m_metaDataAlbum = album;
}

void EncoderWave::initStream() {

    // Tell the encoder to automatically convert float input range to the correct output range.
    sf_command(m_pSndfile, SFC_SET_NORM_FLOAT, nullptr, SF_TRUE);
    // Tell the encoder that, when converting to integer formats, clip
    // automatically the values that go outside of the allowed range.
    // Warning! Depending on how libsndfile is compiled autoclip may not work.
    // Ensure CPU_CLIPS_NEGATIVE and CPU_CLIPS_POSITIVE is setup properly in the build.
    sf_command(m_pSndfile, SFC_SET_CLIPPING, nullptr, SF_TRUE);

    // Strings passed to and retrieved from sf_get_string/sf_set_string are assumed to be utf-8.
    // However, while formats like Ogg/Vorbis and FLAC fully support utf-8, others like WAV and
    // AIFF officially only support ASCII. Writing utf-8 strings to WAV and AIF files with
    // libsndfile will work when read back with libsndfile, but may not work with other programs.
    int ret;
    if (!m_metaDataTitle.isEmpty()) {
        ret = sf_set_string(m_pSndfile, SF_STR_TITLE, m_metaDataTitle.toUtf8().constData());
        if (ret != 0) {
            qWarning("libsndfile error: %s", sf_error_number(ret));
        }
    }

    if (!m_metaDataArtist.isEmpty()) {
        ret = sf_set_string(m_pSndfile, SF_STR_ARTIST, m_metaDataArtist.toUtf8().constData());
        if (ret != 0) {
            qWarning("libsndfile error: %s", sf_error_number(ret));
        }
    }
    if (!m_metaDataAlbum.isEmpty()) {
        int strType = SF_STR_ALBUM;
        if (m_sfInfo.format == SF_FORMAT_AIFF) {
            // There is no AIFF text chunk for "Album". But libsndfile is able to
            // write the SF_STR_COMMENT string into the text chunk with id "ANNO".
            strType = SF_STR_COMMENT;
        }
        ret = sf_set_string(m_pSndfile, strType, m_metaDataAlbum.toUtf8().constData());
        if (ret != 0) {
            qWarning("libsndfile error: %s", sf_error_number(ret));
        }
    }
}

int EncoderWave::initEncoder(mixxx::audio::SampleRate sampleRate, QString* pUserErrorMessage) {
    Q_UNUSED(pUserErrorMessage);
    // set sfInfo.
    // m_sfInfo.format is setup on setEncoderSettings previous to calling initEncoder.
    m_sfInfo.samplerate = sampleRate;
    m_sfInfo.channels = 2;
    m_sfInfo.frames = 0;
    m_sfInfo.sections = 0;
    m_sfInfo.seekable = 0;

    // Opens a soundfile from a virtual file I/O context which is provided by the caller.
    // This is usually used to interface libsndfile to a stream or buffer based system.
    // Apart from the sfvirtual and the user_data parameters this function behaves like sf_open.
    m_pSndfile = sf_open_virtual (&m_virtualIo, SFM_WRITE, &m_sfInfo, m_pCallback) ;

    int ret=0;
    if (m_pSndfile == nullptr) {
        qDebug()
                << "Error initializing Wave encoding. sf_open_virtual returned:"
                << sf_strerror(nullptr);
        ret = -1;
    } else {
        initStream();
    };
    return ret;
}
