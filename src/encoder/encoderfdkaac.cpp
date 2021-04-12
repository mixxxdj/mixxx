#include "encoder/encoderfdkaac.h"

#ifdef __APPLE__
#include <QCoreApplication>
#endif
#include <QDir>
#include <QStandardPaths>
#include <QString>
#include <QStringList>

#include "engine/sidechain/enginesidechain.h"
#include "recording/defs_recording.h"
#include "util/logger.h"
#include "util/sample.h"

namespace {
// recommended in encoder documentation, section 2.4.1
const int kOutBufferBits = 6144;
const mixxx::Logger kLogger("EncoderFdkAac");
} // namespace

EncoderFdkAac::EncoderFdkAac(EncoderCallback* pCallback)
        : aacEncOpen(nullptr),
          aacEncClose(nullptr),
          aacEncEncode(nullptr),
          aacEncInfo(nullptr),
          aacEncoder_SetParam(nullptr),
          m_aacAot(AOT_AAC_LC),
          m_bitrate(0),
          m_channels(0),
          m_samplerate(0),
          m_pCallback(pCallback),
          m_pLibrary(nullptr),
          m_pInputFifo(nullptr),
          m_pFifoChunkBuffer(nullptr),
          m_readRequired(0),
          m_aacEnc(),
          m_pAacDataBuffer(nullptr),
          m_aacInfo(),
          m_hasSbr(false) {
    // Load the shared library
    //
    // Libraries from external sources take priority because they may include HE-AAC support,
    // but we do not risk shipping that with Mixxx because of patents and GPL compatibility.
    //
    // From https://bugzilla.redhat.com/show_bug.cgi?id=1501522#c112 :
    // The Fedora Project is aware that the Free Software Foundation
    // has stated that the Fraunhofer FDK AAC license is GPL
    // incompatible, specifically, because of Clause 3.
    //
    // We believe that the fdk-aac software codec implementation that we
    // wish to include in Fedora [which is shipped with Mixxx on Windows and macOS]
    // is no longer encumbered by AAC patents.
    // This fact means that Clause 3 in the FDK AAC license is a "no op",
    // or to put it plainly, if no patents are in play, there are no
    // patent licenses to disclaim. For this (and only this) specific
    // implementation of fdk-aac, we believe that the FDK AAC license is
    // GPL compatible.
    QStringList libnames;
#if __WINDOWS__
    // Search for library from B.U.T.T.
    QString buttFdkAacPath = buttWindowsFdkAac();
    if (!buttFdkAacPath.isEmpty()) {
        kLogger.debug() << "Found libfdk-aac at" << buttFdkAacPath;
        libnames << buttFdkAacPath;
    }
#elif __APPLE__
    // Homebrew
    libnames << QStringLiteral("/usr/local/lib/libfdk-aac");
    // MacPorts
    libnames << QStringLiteral("/opt/local/lib/libfdk-aac");

    // Mixxx application bundle
    QFileInfo bundlePath(QCoreApplication::applicationDirPath() +
            QStringLiteral("/../Frameworks/libfdk-aac"));
    libnames << bundlePath.absoluteFilePath();
#endif
    libnames << QStringLiteral("fdk-aac");
    // Although the line above should suffice, detection of the fdk-aac library
    // does not work on Ubuntu 18.04 LTS and Ubuntu 20.04 LTS:
    //
    //     $ dpkg -L libfdk-aac1 | grep so
    //     /usr/lib/x86_64-linux-gnu/libfdk-aac.so.1.0.0
    //     /usr/lib/x86_64-linux-gnu/libfdk-aac.so.1
    //
    // As a workaround, we have to hardcode the filenames here.
    libnames << QStringLiteral("libfdk-aac.so.1")
             << QStringLiteral("libfdk-aac.so.2");

    QStringList errorMessages;
    for (const auto& libname : qAsConst(libnames)) {
        m_pLibrary = std::make_unique<QLibrary>(libname, 2);
        if (m_pLibrary->load()) {
            kLogger.debug() << "Successfully loaded encoder library" << m_pLibrary->fileName();
            break;
        }
        // The APIs this class uses did not change between library versions 1 and 2.
        // Ubuntu 20.04 LTS has version 1.
        m_pLibrary = std::make_unique<QLibrary>(libname, 1);
        if (m_pLibrary->load()) {
            kLogger.debug() << "Successfully loaded encoder library" << m_pLibrary->fileName();
            break;
        }
        // collect error messages for the case we have no success
        errorMessages.append(m_pLibrary->errorString());
        m_pLibrary = nullptr;
    }

    if (!m_pLibrary || !m_pLibrary->isLoaded()) {
        for (const auto& errorMessage : qAsConst(errorMessages)) {
            kLogger.warning() << "Failed to load AAC encoder library:" << errorMessage;
        }
        return;
    }

    aacEncGetLibInfo = (aacEncGetLibInfo_)m_pLibrary->resolve("aacEncGetLibInfo");
    aacEncOpen = (aacEncOpen_)m_pLibrary->resolve("aacEncOpen");
    aacEncClose = (aacEncClose_)m_pLibrary->resolve("aacEncClose");
    aacEncEncode = (aacEncEncode_)m_pLibrary->resolve("aacEncEncode");
    aacEncInfo = (aacEncInfo_)m_pLibrary->resolve("aacEncInfo");
    aacEncoder_SetParam = (aacEncoder_SetParam_)m_pLibrary->resolve("aacEncoder_SetParam");

    // Check if all function pointers aren't null.
    // Otherwise, the version of libfdk-aac loaded doesn't comply with the official distribution
    if (!aacEncGetLibInfo ||
            !aacEncOpen ||
            !aacEncClose ||
            !aacEncEncode ||
            !aacEncInfo ||
            !aacEncoder_SetParam) {
        m_pLibrary->unload();
        m_pLibrary = nullptr;

        kLogger.warning() << "Failed to load AAC encoder library: Interface of"
                          << m_pLibrary->fileName() << "is not as expected";

        kLogger.debug() << "aacEncGetLibInfo:" << aacEncGetLibInfo;
        kLogger.debug() << "aacEncOpen:" << aacEncOpen;
        kLogger.debug() << "aacEncClose:" << aacEncClose;
        kLogger.debug() << "aacEncEncode:" << aacEncEncode;
        kLogger.debug() << "aacEncInfo:" << aacEncInfo;
        kLogger.debug() << "aacEncoder_SetParam:" << aacEncoder_SetParam;
        return;
    }

    LIB_INFO libinfo[FDK_MODULE_LAST] = {};
    aacEncGetLibInfo(libinfo);
    for (const auto& li : libinfo) {
        if (li.module_id == FDK_NONE) {
            break;
        }
        qDebug() << li.title << li.versionStr;
        if (li.module_id == FDK_SBRENC) {
            m_hasSbr = true;
        }
    }
}

EncoderFdkAac::~EncoderFdkAac() {
    if (m_pLibrary && m_pLibrary->isLoaded()) {
        aacEncClose(&m_aacEnc);

        flush();
        m_pLibrary->unload();
        kLogger.debug() << "Unloaded libfdk-aac";
    }

    delete[] m_pAacDataBuffer;
    delete m_pFifoChunkBuffer;
    delete m_pInputFifo;
}

QString EncoderFdkAac::buttWindowsFdkAac() {
    // Return %APPDATA%/Local path
    QString appData = QStandardPaths::writableLocation(
            QStandardPaths::AppLocalDataLocation);
    appData = QFileInfo(appData).absolutePath() + "/..";

    // Candidate paths for a butt installation
    QStringList searchPaths;
    searchPaths << "C:/Program Files";
    searchPaths << "C:/Program Files (x86)";
    searchPaths << appData;

    // Try to find a butt installation in one of the
    // potential paths above
    for (const auto& topPath : qAsConst(searchPaths)) {
        QDir folder(topPath);
        if (!folder.exists()) {
            continue;
        }

        // Typical name for a butt installation folder
        // is "butt-x.x.x" so list subfolders beginning with "butt"
        QStringList nameFilters("butt*");
        QStringList subfolders =
                folder.entryList(nameFilters, QDir::Dirs, QDir::Name);

        // If a butt installation is found, try
        // to find libfdk-aac in it
        for (const auto& subName : qAsConst(subfolders)) {
            if (!folder.cd(subName)) {
                continue;
            }

            kLogger.debug()
                    << "Found potential B.U.T.T installation at"
                    << (topPath + "/" + subName);

            QString libFile = "libfdk-aac-2.dll";
            if (folder.exists(libFile)) {
                // Found a libfdk-aac here.
                // Return the full path of the .dll file.
                return folder.absoluteFilePath(libFile);
            }

            folder.cdUp();
        }
    }

    return QString();
}

void EncoderFdkAac::setEncoderSettings(const EncoderSettings& settings) {
    if (settings.getFormat() == ENCODING_AAC) {
        // MPEG-4 AAC-LC
        m_aacAot = AOT_AAC_LC;
    } else if (settings.getFormat() == ENCODING_HEAAC) {
        // MPEG-4 HE-AAC
        m_aacAot = AOT_SBR;
    } else if (settings.getFormat() == ENCODING_HEAACV2) {
        // MPEG-4 HE-AACv2
        m_aacAot = AOT_PS;
    } else {
        // Fallback to AAC-LC in case
        // of unknown value
        m_aacAot = AOT_AAC_LC;
    }

    // TODO(Palakis): support more bitrate configurations
    m_bitrate = settings.getQuality();
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

int EncoderFdkAac::initEncoder(int samplerate, QString* pUserErrorMessage) {
    m_samplerate = samplerate;

    if (!m_pLibrary) {
        kLogger.warning() << "initEncoder failed: fdk-aac library not loaded";
        if (pUserErrorMessage) {
            // TODO(Palakis): write installation guide on Mixxx's wiki
            // and include link in message below
            *pUserErrorMessage = QObject::tr(
                    "<html>Mixxx cannot record or stream in AAC "
                    "or HE-AAC without the FDK-AAC encoder. "
                    "In order to record or stream in AAC or AAC+, you need to "
                    "download <b>libfdk-aac</b> "
                    "and install it on your system.");
        }
        return -1;
    }

    if ((m_aacAot & AOT_SBR) == AOT_SBR && !m_hasSbr) {
        kLogger.warning() << "initEncoder failed: fdk-aac library has no HE-AAC support";
        if (pUserErrorMessage) {
            *pUserErrorMessage = QObject::tr(
                    "The installed AAC encoding library does not support "
                    "HE-AAC, only plain AAC. Configure a different encoding "
                    "format in the preferences.");
        }
        return -1;
    }

    // This initializes the encoder handle but not the encoder itself.
    // Actual encoder init is done below.
    aacEncOpen(&m_aacEnc, 0, m_channels);
    m_pAacDataBuffer = new unsigned char[kOutBufferBits * m_channels]();

    // AAC Object Type: specifies "mode": AAC-LC, HE-AAC, HE-AACv2, DAB AAC, etc...
    if (aacEncoder_SetParam(m_aacEnc, AACENC_AOT, m_aacAot) != AACENC_OK) {
        kLogger.warning() << "aac encoder setting AOT failed!";
        return -1;
    }

    // Input audio samplerate
    if (aacEncoder_SetParam(m_aacEnc, AACENC_SAMPLERATE, m_samplerate) != AACENC_OK) {
        kLogger.warning() << "aac encoder setting samplerate failed!";
        return -1;
    }
    // Input and output audio channel count
    if (aacEncoder_SetParam(m_aacEnc, AACENC_CHANNELMODE, m_channels) != AACENC_OK) {
        kLogger.warning() << "aac encoder setting channel mode failed!";
        return -1;
    }
    // Input audio channel order (fixed to 1 for traditional WAVE ordering: L, R, ...)
    if (aacEncoder_SetParam(m_aacEnc, AACENC_CHANNELORDER, 1) != AACENC_OK) {
        kLogger.warning() << "aac encoder setting channel order failed!";
        return -1;
    }

    // Output bitrate in bits per second
    // m_bitrate is in kilobits per second, conversion needed
    if (aacEncoder_SetParam(m_aacEnc, AACENC_BITRATE, m_bitrate * 1000) != AACENC_OK) {
        kLogger.warning() << "aac encoder setting bitrate failed!";
        return -1;
    }
    // Transport type (2 = ADTS)
    if (aacEncoder_SetParam(m_aacEnc, AACENC_TRANSMUX, 2) != AACENC_OK) {
        kLogger.warning() << "aac encoder setting transmux failed!";
        return -1;
    }
    // Enable the AAC Afterburner, which increases audio quality
    // at the cost of increased CPU and memory usage.
    // Fraunhofer recommends to enable this if increased CPU and memory
    // consumption is not a problem.
    // TODO(Palakis): is this an issue even with 12-year old computers
    // and notebooks?
    if (aacEncoder_SetParam(m_aacEnc, AACENC_AFTERBURNER, 1) != AACENC_OK) {
        kLogger.warning() << "aac encoder setting adterburner failed!";
        return -1;
    }

    // Actual encoder init, validates settings provided above
    int result = aacEncEncode(m_aacEnc, nullptr, nullptr, nullptr, nullptr);
    if (result != AACENC_OK) {
        kLogger.warning() << "aac encoder init failed! error code:" << result;
        return -1;
    }

    aacEncInfo(m_aacEnc, &m_aacInfo);
    m_readRequired = m_aacInfo.frameLength * m_channels;

    // Size the input FIFO buffer with twice the maximum possible sample count that can be
    // processed at once, to avoid skipping frames or waiting for the required sample count
    // and encode at a regular pace.
    // This is set to the buffer size of the sidechain engine because
    // Recording (which uses this engine) sends more samples at once to the encoder than
    // the Live Broadcasting implementation
    m_pInputFifo = new FIFO<SAMPLE>(EngineSideChain::SIDECHAIN_BUFFER_SIZE * 2);

    m_pFifoChunkBuffer = new SAMPLE[m_readRequired * sizeof(SAMPLE)]();
    return 0;
}

void EncoderFdkAac::encodeBuffer(const CSAMPLE* samples, const int sampleCount) {
    if (!m_pInputFifo) {
        return;
    }
    int writeCount = sampleCount;
    int writeAvailable = m_pInputFifo->writeAvailable();
    if (writeCount > writeAvailable) {
        kLogger.warning() << "FIFO buffer too small, loosing samples!"
                          << "required:" << writeCount
                          << "; available: " << writeAvailable;
        writeCount = writeAvailable;
    }
    if (writeCount > 0) {
        SAMPLE* dataPtr1;
        ring_buffer_size_t size1;
        SAMPLE* dataPtr2;
        ring_buffer_size_t size2;
        // We use size1 and size2, so we can ignore the return value
        (void)m_pInputFifo->aquireWriteRegions(writeCount, &dataPtr1, &size1, &dataPtr2, &size2);
        // fdk-aac doesn't support float samples, so convert
        // to integers instead
        SampleUtil::convertFloat32ToS16(dataPtr1, samples, size1);
        if (size2 > 0) {
            SampleUtil::convertFloat32ToS16(dataPtr2, samples + size1, size2);
        }
        m_pInputFifo->releaseWriteRegions(writeCount);
    }
    processFIFO();
}

void EncoderFdkAac::processFIFO() {
    if (!m_pInputFifo || !m_pFifoChunkBuffer) {
        return;
    }

    while (m_pInputFifo->readAvailable() >= m_readRequired) {
        m_pInputFifo->read(m_pFifoChunkBuffer, m_readRequired);

        // fdk-aac only accept pointers for most buffer settings.
        // Declare settings here and point to them below.
        int inSampleSize = sizeof(SAMPLE);
        int inDataSize = m_readRequired * inSampleSize;
        int inDataDescription = IN_AUDIO_DATA;

        int outElemSize = sizeof(unsigned char);
        int outDataSize = kOutBufferBits * m_channels * outElemSize;
        int outDataDescription = OUT_BITSTREAM_DATA;

        // Input Buffer
        AACENC_BufDesc inputBuf;
        inputBuf.numBufs = 1;
        inputBuf.bufs = (void**)&m_pFifoChunkBuffer;
        inputBuf.bufSizes = &inDataSize;
        inputBuf.bufElSizes = &inSampleSize;
        inputBuf.bufferIdentifiers = &inDataDescription;

        AACENC_InArgs inputDesc;
        inputDesc.numInSamples = m_readRequired;
        inputDesc.numAncBytes = 0;

        // Output (result) Buffer
        AACENC_BufDesc outputBuf;
        outputBuf.numBufs = 1;
        outputBuf.bufs = (void**)&m_pAacDataBuffer;
        outputBuf.bufSizes = &outDataSize;
        outputBuf.bufElSizes = &outElemSize;
        outputBuf.bufferIdentifiers = &outDataDescription;

        // Populated by aacEncEncode
        AACENC_OutArgs outputDesc;

        int result = aacEncEncode(m_aacEnc, &inputBuf, &outputBuf, &inputDesc, &outputDesc);
        if (result != AACENC_OK) {
            kLogger.warning() << "aacEncEncode failed! error code:" << result;
            return;
        }

        int sampleDiff = inputDesc.numInSamples - outputDesc.numInSamples;
        if (sampleDiff > 0) {
            kLogger.warning() << "encoder ignored" << sampleDiff << "samples!";
        }

        m_pCallback->write(nullptr, m_pAacDataBuffer, 0, outputDesc.numOutBytes);
    }
}

void EncoderFdkAac::updateMetaData(
        const QString& artist, const QString& title, const QString& album) {
    (void)artist, (void)title, (void)album;
}

void EncoderFdkAac::flush() {
    // At this point there may still be samples in the FIFO buffer.
    processFIFO();
}
