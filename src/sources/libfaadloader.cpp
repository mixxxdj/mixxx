#include "sources/libfaadloader.h"
#include "util/logger.h"

namespace {

mixxx::Logger kLogger("LibFaadLoader");

} // anonymous namespace

// static
LibFaadLoader* LibFaadLoader::Instance() {
    static LibFaadLoader libFaadLoader;
    return &libFaadLoader;
}

LibFaadLoader::LibFaadLoader()
        : m_neAACDecOpen(nullptr),
          m_neAACDecGetCurrentConfiguration(nullptr),
          m_neAACDecSetConfiguration(nullptr),
          m_neAACDecInit2(nullptr),
          m_neAACDecClose(nullptr),
          m_neAACDecPostSeekReset(nullptr),
          m_neAACDecDecode2(nullptr),
          m_neAACDecGetErrorMessage(nullptr) {
    // Load shared library
    QStringList libnames;
#ifdef __WINDOWS__
    // http://www.rarewares.org/aac-decoders.php
    libnames << "libfaad2.dll";
#elif __APPLE__
    // First try default location
    libnames << "libfaad2.dylib";
    // Using Homebrew ('brew install faad2' command):
    libnames << "/usr/local/lib/libfaad2.dylib";
    // Using MacPorts ('sudo port install faad2' command):
    libnames << "/opt/local/lib/libfaad2.dylib";
#else
    libnames << "libfaad.so";
#endif

    for (const auto& libname : libnames) {
        m_pLibrary.reset();
        m_pLibrary = std::make_unique<QLibrary>(libname, 0);
        if (m_pLibrary->load()) {
            break;
        }
    }

    if (!m_pLibrary->isLoaded()) {
        kLogger.warning() << "Failed to load " << libnames << ", " << m_pLibrary->errorString();
        m_pLibrary.reset();
        return;
    }

    m_neAACDecOpen = reinterpret_cast<NeAACDecOpen_t>(
            m_pLibrary->resolve("NeAACDecOpen"));
    m_neAACDecGetCurrentConfiguration = reinterpret_cast<NeAACDecGetCurrentConfiguration_t>(
            m_pLibrary->resolve("NeAACDecGetCurrentConfiguration"));
    m_neAACDecSetConfiguration = reinterpret_cast<NeAACDecSetConfiguration_t>(
            m_pLibrary->resolve("NeAACDecSetConfiguration"));
    m_neAACDecInit2 = reinterpret_cast<NeAACDecInit2_t>(
            m_pLibrary->resolve("NeAACDecInit2"));
    m_neAACDecClose = reinterpret_cast<NeAACDecClose_t>(
            m_pLibrary->resolve("NeAACDecClose"));
    m_neAACDecPostSeekReset = reinterpret_cast<NeAACDecPostSeekReset_t>(
            m_pLibrary->resolve("NeAACDecPostSeekReset"));
    m_neAACDecDecode2 = reinterpret_cast<NeAACDecDecode2_t>(
            m_pLibrary->resolve("NeAACDecDecode2"));
    m_neAACDecGetErrorMessage = reinterpret_cast<NeAACDecGetErrorMessage_t>(
            m_pLibrary->resolve("NeAACDecGetErrorMessage"));

    if (!m_neAACDecOpen ||
            !m_neAACDecGetCurrentConfiguration ||
            !m_neAACDecSetConfiguration ||
            !m_neAACDecInit2 ||
            !m_neAACDecClose ||
            !m_neAACDecPostSeekReset ||
            !m_neAACDecDecode2 ||
            !m_neAACDecGetErrorMessage) {
        kLogger.debug() << "NeAACDecOpen:" << m_neAACDecOpen;
        kLogger.debug() << "NeAACDecGetCurrentConfiguration:" << m_neAACDecGetCurrentConfiguration;
        kLogger.debug() << "NeAACDecSetConfiguration:" << m_neAACDecSetConfiguration;
        kLogger.debug() << "NeAACDecInit2:" << m_neAACDecInit2;
        kLogger.debug() << "NeAACDecClose:" << m_neAACDecClose;
        kLogger.debug() << "NeAACDecPostSeekReset:" << m_neAACDecPostSeekReset;
        kLogger.debug() << "NeAACDecDecode2:" << m_neAACDecDecode2;
        kLogger.debug() << "NeAACDecGetErrorMessage:" << m_neAACDecGetErrorMessage;
        m_neAACDecOpen = nullptr;
        m_neAACDecGetCurrentConfiguration = nullptr;
        m_neAACDecSetConfiguration = nullptr;
        m_neAACDecInit2 = nullptr;
        m_neAACDecClose = nullptr;
        m_neAACDecPostSeekReset = nullptr;
        m_neAACDecDecode2 = nullptr;
        m_neAACDecGetErrorMessage = nullptr;
        m_pLibrary->unload();
        m_pLibrary.reset();
        return;
    }
    kLogger.info() << "Successfully loaded library" << m_pLibrary->fileName();
};

bool LibFaadLoader::isLoaded() {
    if (m_pLibrary) {
        return m_pLibrary->isLoaded();
    }
    return false;
}

LibFaadLoader::Handle LibFaadLoader::Open() {
    if (m_neAACDecOpen) {
        return m_neAACDecOpen();
    }
    return nullptr;
}

LibFaadLoader::Configuration*
LibFaadLoader::GetCurrentConfiguration(Handle hDecoder) {
    if (m_neAACDecGetCurrentConfiguration) {
        return m_neAACDecGetCurrentConfiguration(hDecoder);
    }
    return nullptr;
}

unsigned char LibFaadLoader::SetConfiguration(
        Handle hDecoder, Configuration* pConfig) {
    if (m_neAACDecSetConfiguration) {
        return m_neAACDecSetConfiguration(hDecoder, pConfig);
    }
    // Return values:
    // 0 – Error, invalid configuration.
    // 1 – OK
    return 0;
}

// Init the library using a DecoderSpecificInfo
char LibFaadLoader::Init2(
        Handle hDecoder,
        unsigned char* pBuffer,
        unsigned long SizeOfDecoderSpecificInfo,
        unsigned long* pSamplerate,
        unsigned char* pChannels) {
    if (m_neAACDecInit2) {
        return m_neAACDecInit2(hDecoder,
                pBuffer,
                SizeOfDecoderSpecificInfo,
                pSamplerate,
                pChannels);
    }
    // Return values:
    // < 0 – Error
    // 0 - OK
    return -1;
}

void LibFaadLoader::Close(Handle hDecoder) {
    if (m_neAACDecClose) {
        m_neAACDecClose(hDecoder);
    }
}

void LibFaadLoader::PostSeekReset(Handle hDecoder, long frame) {
    if (m_neAACDecPostSeekReset) {
        m_neAACDecPostSeekReset(hDecoder, frame);
    }
}

void* LibFaadLoader::Decode2(
        Handle hDecoder,
        FrameInfo* pInfo,
        unsigned char* pBuffer,
        unsigned long bufferSize,
        void** ppSampleBuffer,
        unsigned long sampleBufferSize) {
    if (m_neAACDecDecode2) {
        return m_neAACDecDecode2(
                hDecoder,
                pInfo,
                pBuffer,
                bufferSize,
                ppSampleBuffer,
                sampleBufferSize);
    }
    return nullptr;
}

char* LibFaadLoader::GetErrorMessage(unsigned char errcode) {
    if (m_neAACDecGetErrorMessage) {
        return m_neAACDecGetErrorMessage(errcode);
    }
    return nullptr;
}
