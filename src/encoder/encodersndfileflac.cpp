#include "encoder/encodersndfileflac.h"

#include <QtDebug>

#include "encoder/encodercallback.h"
#include "util/sample.h"

namespace {
constexpr SINT kEncBufferSize = 8192; // used inside libsndfile for flac
constexpr CSAMPLE_GAIN kFloatToIntConversionFactor = INT_MIN * -1.0f;

//static
void convertFloat32ToIntFormat(int* pDest, const CSAMPLE* pSrc, SINT numSamples, int format) {
    if (format & SF_FORMAT_PCM_16) {
        // note: LOOP VECTORIZED"
        for (SINT i = 0; i < numSamples; ++i) {
            pDest[i] = static_cast<int>(math_clamp(pSrc[i] * kFloatToIntConversionFactor,
                    static_cast<CSAMPLE>(static_cast<int>(INT_MIN & 0xFFFF0000)),
                    static_cast<CSAMPLE>(static_cast<int>(INT_MAX & 0xFFFF0000))));
        }
    } else if (format & SF_FORMAT_PCM_24) {
        // note: LOOP VECTORIZED"
        for (SINT i = 0; i < numSamples; ++i) {
            pDest[i] = static_cast<int>(math_clamp(pSrc[i] * kFloatToIntConversionFactor,
                    static_cast<CSAMPLE>(static_cast<int>(INT_MIN & 0xFFFFFF00)),
                    static_cast<CSAMPLE>(static_cast<int>(INT_MAX & 0xFFFFFF00))));
        }
    } else {
        DEBUG_ASSERT(!"Not implemented");
    }
}

} // namespace

EncoderSndfileFlac::EncoderSndfileFlac(EncoderCallback* pCallback)
        : EncoderWave(pCallback),
          m_compression(0) {
}

void EncoderSndfileFlac::setEncoderSettings(const EncoderSettings& settings)
{
    m_sfInfo.format = SF_FORMAT_FLAC;

    int radio = settings.getSelectedOption(EncoderFlacSettings::BITS_GROUP);
    switch(radio) {
        case 0:
            m_sfInfo.format |= SF_FORMAT_PCM_16;
            break;
        case 1:
            m_sfInfo.format |= SF_FORMAT_PCM_24;
            break;
        default:
            m_sfInfo.format |= SF_FORMAT_PCM_16;
            qWarning() << " Unexpected radio index on setEncoderSettings: "
                       << radio << ". reverting to Flac 16bits";
            break;
    }

    m_compression = static_cast<double>(settings.getCompression()) / 8.0;
}

void EncoderSndfileFlac::encodeBuffer(const CSAMPLE* pBuffer, const int iBufferSize) {
    if (m_pClampBuffer) {
        SINT numSamplesLeft = iBufferSize;
        while (numSamplesLeft > 0) {
            const SINT numSamplesToWrite = math_min(numSamplesLeft, kEncBufferSize);
            convertFloat32ToIntFormat(m_pClampBuffer.get(),
                    pBuffer,
                    numSamplesToWrite,
                    m_sfInfo.format);
            sf_write_int(m_pSndfile, m_pClampBuffer.get(), numSamplesToWrite);
            pBuffer += numSamplesToWrite;
            numSamplesLeft -= numSamplesToWrite;
        }
    } else {
        sf_write_float(m_pSndfile, pBuffer, iBufferSize);
    }
}

void EncoderSndfileFlac::initStream() {
    EncoderWave::initStream();
#if defined SFC_SUPPORTS_SET_COMPRESSION_LEVEL // Seems that this only exists since version 1.0.26
    // Tell the compression setting to use.
    sf_command(m_pSndfile, SFC_SET_COMPRESSION_LEVEL, &m_compression, sizeof(double));
#endif //SFC_SUPPORTS_SET_COMPRESSION_LEVEL

    // Version 1.0.28 suffers broken clamping https://bugs.launchpad.net/mixxx/+bug/1915298
    // We receive "libsndfile-1.0.28" on Ubuntu Bionic 18.04 LTS/Focal 20.04 LTS/Grovy 20.10
    // Older versions are not expected. All newer version have a working internal clamping
    const char* sf_version = sf_version_string();
    if (strstr(sf_version, "-1.0.28") != nullptr) {
        m_pClampBuffer = std::make_unique<int[]>(kEncBufferSize);
        sf_command(m_pSndfile, SFC_SET_CLIPPING, nullptr, SF_FALSE);
    }
}
