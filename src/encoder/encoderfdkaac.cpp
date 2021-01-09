#include "encoder/encoderfdkaac.h"

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
          m_library(nullptr),
          m_pInputFifo(nullptr),
          m_pFifoChunkBuffer(nullptr),
          m_readRequired(0),
          m_aacEnc(),
          m_pAacDataBuffer(nullptr),
          m_aacInfo() {
    // Load shared library
    // Code import from encodermp3.cpp
    QStringList libnames;
    QString libname = "";
#ifdef __LINUX__
    libnames << "fdk-aac";
    libnames << "libfdk-aac.so.1";
#elif __WINDOWS__
    // Give top priority to libfdk-aac copied
    // into Mixxx's installation folder
    libnames << "libfdk-aac-1.dll";

    // Fallback and user-friendly method: use libfdk-aac
    // provided with B.U.T.T installed in
    // a standard location
    QString buttFdkAacPath = buttWindowsFdkAac();
    if (!buttFdkAacPath.isNull()) {
        kLogger.debug() << "Found libfdk-aac at" << buttFdkAacPath;
        libnames << buttFdkAacPath;
    }

    // Last resort choices: try versions with unusual names
    libnames << "libfdk-aac.dll";
    libnames << "libfdkaac.dll";
#elif __APPLE__
    // Using Homebrew ('brew install fdk-aac' command):
    libnames << "/usr/local/lib/libfdk-aac.dylib";
    // Using MacPorts ('sudo port install libfdk-aac' command):
    libnames << "/opt/local/lib/libfdk-aac.dylib";
#endif

    for (const auto& libname : libnames) {
        m_library = new QLibrary(libname);
        if (m_library->load()) {
            kLogger.debug() << "Successfully loaded encoder library " << m_library->fileName();
            break;
        } else {
            kLogger.warning() << "Failed to load " << libname << ", " << m_library->errorString();
        }

        delete m_library;
        m_library = nullptr;
    }

    if (!m_library || !m_library->isLoaded()) {
        ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
        props->setType(DLG_WARNING);
        props->setTitle(QObject::tr("Encoder"));

        // TODO(Palakis): write installation guide on Mixxx's wiki
        // and include link in message below
        QString missingCodec = QObject::tr(
                "<html>Mixxx cannot record or stream in AAC "
                "or AAC+ without the FDK-AAC encoder. Due to licensing issues, "
                "we cannot distribute this with Mixxx. "
                "In order to record or stream in AAC or AAC+, you need to "
                "download <b>libfdk-aac</b> "
                "and install it on your system.");

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

    aacEncGetLibInfo = (aacEncGetLibInfo_)m_library->resolve("aacEncGetLibInfo");
    aacEncOpen = (aacEncOpen_)m_library->resolve("aacEncOpen");
    aacEncClose = (aacEncClose_)m_library->resolve("aacEncClose");
    aacEncEncode = (aacEncEncode_)m_library->resolve("aacEncEncode");
    aacEncInfo = (aacEncInfo_)m_library->resolve("aacEncInfo");
    aacEncoder_SetParam = (aacEncoder_SetParam_)m_library->resolve("aacEncoder_SetParam");

    // Check if all function pointers aren't null.
    // Otherwise, the version of libfdk-aac loaded doesn't comply with the official distribution
    // Shouldn't happen on Linux, mainly on Windows.
    if (!aacEncGetLibInfo ||
            !aacEncOpen ||
            !aacEncClose ||
            !aacEncEncode ||
            !aacEncInfo ||
            !aacEncoder_SetParam) {
        m_library->unload();
        delete m_library;
        m_library = nullptr;

        kLogger.debug() << "aacEncGetLibInfo:" << aacEncGetLibInfo;
        kLogger.debug() << "aacEncOpen:" << aacEncOpen;
        kLogger.debug() << "aacEncClose:" << aacEncClose;
        kLogger.debug() << "aacEncEncode:" << aacEncEncode;
        kLogger.debug() << "aacEncInfo:" << aacEncInfo;
        kLogger.debug() << "aacEncoder_SetParam:" << aacEncoder_SetParam;

        ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
        props->setType(DLG_WARNING);
        props->setTitle(QObject::tr("Encoder"));
        QString key = QObject::tr(
                "<html>Mixxx has detected that you use a modified version of libfdk-aac. "
                "See <a href='http://mixxx.org/wiki/doku.php/internet_broadcasting'>Mixxx Wiki</a> "
                "for more information.</html>");
        props->setText(key);
        props->setKey(key);
        ErrorDialogHandler::instance()->requestErrorDialog(props);
        return;
    }

    kLogger.debug() << "Loaded libfdk-aac";
}

EncoderFdkAac::~EncoderFdkAac() {
    if (m_library && m_library->isLoaded()) {
        aacEncClose(&m_aacEnc);

        flush();
        m_library->unload();
        delete m_library;
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
    for (QString topPath : searchPaths) {
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
        for (QString subName : subfolders) {
            if (!folder.cd(subName)) {
                continue;
            }

            kLogger.debug()
                    << "Found potential B.U.T.T installation at"
                    << (topPath + "/" + subName);

            QString libFile = "libfdk-aac-1.dll";
            if (folder.exists(libFile)) {
                // Found a libfdk-aac here.
                // Return the full path of the .dll file.
                return folder.absoluteFilePath(libFile);
            }

            folder.cdUp();
        }
    }

    return QString::null;
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

int EncoderFdkAac::initEncoder(int samplerate, QString& errorMessage) {
    (void)errorMessage;
    m_samplerate = samplerate;

    if (!m_library) {
        kLogger.warning() << "initEncoder failed: fdk-aac library not loaded";
        return -1;
    }

    // This initializes the encoder handle but not the encoder itself.
    // Actual encoder init is done below.
    aacEncOpen(&m_aacEnc, 0, m_channels);
    m_pAacDataBuffer = new unsigned char[kOutBufferBits * m_channels]();

    // AAC Object Type: specifies "mode": AAC-LC, HE-AAC, HE-AACv2, DAB AAC, etc...
    aacEncoder_SetParam(m_aacEnc, AACENC_AOT, m_aacAot);

    // Input audio samplerate
    aacEncoder_SetParam(m_aacEnc, AACENC_SAMPLERATE, m_samplerate);
    // Input and output audio channel count
    aacEncoder_SetParam(m_aacEnc, AACENC_CHANNELMODE, m_channels);
    // Input audio channel order (fixed to 1 for traditional WAVE ordering: L, R, ...)
    aacEncoder_SetParam(m_aacEnc, AACENC_CHANNELORDER, 1);

    // Output bitrate in bits per second
    // m_bitrate is in kilobits per second, conversion needed
    aacEncoder_SetParam(m_aacEnc, AACENC_BITRATE, m_bitrate * 1000);
    // Transport type (2 = ADTS)
    aacEncoder_SetParam(m_aacEnc, AACENC_TRANSMUX, 2);
    // Enable the AAC Afterburner, which increases audio quality
    // at the cost of increased CPU and memory usage.
    // Fraunhofer recommends to enable this if increased CPU and memory
    // consumption is not a problem.
    // TODO(Palakis): is this an issue even with 12-year old computers
    // and notebooks?
    aacEncoder_SetParam(m_aacEnc, AACENC_AFTERBURNER, 1);

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

    int writeRequired = sampleCount;
    int writeAvailable = m_pInputFifo->writeAvailable();
    if (writeRequired > writeAvailable) {
        kLogger.warning() << "FIFO buffer too small, loosing samples!"
                          << "required:" << writeRequired
                          << "; available: " << writeAvailable;
    }

    int writeCount = math_min(writeRequired, writeAvailable);
    if (writeCount > 0) {
        // fdk-aac doesn't support float samples, so convert
        // to integers instead
        SAMPLE convertedSamples[writeCount];
        SampleUtil::convertFloat32ToS16(convertedSamples, samples, writeCount);
        m_pInputFifo->write(convertedSamples, writeCount);
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
