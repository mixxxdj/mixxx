#include "sources/libfaadloader.h"

#include "util/logger.h"

namespace {

mixxx::Logger kLogger("faad2::LibLoader");

} // anonymous namespace

namespace faad2 {

// static
LibLoader* LibLoader::Instance() {
    static LibLoader libFaadLoader;
    return &libFaadLoader;
}

LibLoader::LibLoader()
        : m_neAACDecOpen(nullptr),
          m_neAACDecGetCurrentConfiguration(nullptr),
          m_neAACDecSetConfiguration(nullptr),
          m_neAACDecInit(nullptr),
          m_neAACDecInit2(nullptr),
          m_neAACDecClose(nullptr),
          m_neAACDecPostSeekReset(nullptr),
          m_neAACDecDecode2(nullptr),
          m_neAACDecGetErrorMessage(nullptr),
          m_neAACDecGetVersion(nullptr) {
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

    for (const auto& libname : qAsConst(libnames)) {
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
    m_neAACDecInit = reinterpret_cast<NeAACDecInit_t>(
            m_pLibrary->resolve("NeAACDecInit"));
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
    m_neAACDecGetVersion = reinterpret_cast<NeAACDecGetVersion_t>(
            m_pLibrary->resolve("NeAACDecGetVersion"));

    if (!m_neAACDecOpen ||
            !m_neAACDecGetCurrentConfiguration ||
            !m_neAACDecSetConfiguration ||
            !m_neAACDecInit ||
            !m_neAACDecInit2 ||
            !m_neAACDecClose ||
            !m_neAACDecPostSeekReset ||
            !m_neAACDecDecode2 ||
            !m_neAACDecGetErrorMessage) {
        kLogger.warning() << "NeAACDecOpen:" << m_neAACDecOpen;
        kLogger.warning() << "NeAACDecGetCurrentConfiguration:" << m_neAACDecGetCurrentConfiguration;
        kLogger.warning() << "NeAACDecSetConfiguration:" << m_neAACDecSetConfiguration;
        kLogger.warning() << "NeAACDecInit:" << m_neAACDecInit;
        kLogger.warning() << "NeAACDecInit2:" << m_neAACDecInit2;
        kLogger.warning() << "NeAACDecClose:" << m_neAACDecClose;
        kLogger.warning() << "NeAACDecPostSeekReset:" << m_neAACDecPostSeekReset;
        kLogger.warning() << "NeAACDecDecode2:" << m_neAACDecDecode2;
        kLogger.warning() << "NeAACDecGetErrorMessage:" << m_neAACDecGetErrorMessage;
        kLogger.warning() << "NeAACDecGetVersion:" << m_neAACDecGetVersion; // From FAAD 2.8.2
        m_neAACDecOpen = nullptr;
        m_neAACDecGetCurrentConfiguration = nullptr;
        m_neAACDecSetConfiguration = nullptr;
        m_neAACDecInit = nullptr;
        m_neAACDecInit2 = nullptr;
        m_neAACDecClose = nullptr;
        m_neAACDecPostSeekReset = nullptr;
        m_neAACDecDecode2 = nullptr;
        m_neAACDecGetErrorMessage = nullptr;
        m_neAACDecGetVersion = nullptr;
        m_pLibrary->unload();
        m_pLibrary.reset();
        return;
    }
    char* faad_id_string = nullptr;
    char const* version =
            GetVersion(&faad_id_string, nullptr) == 0 ? faad_id_string : "<unknown>";
    kLogger.info()
            << "Successfully loaded FAAD2 library"
            << m_pLibrary->fileName()
            << "version"
            << version;
};

bool LibLoader::isLoaded() const {
    return m_pLibrary && m_pLibrary->isLoaded();
}

DecoderHandle LibLoader::Open() const {
    if (m_neAACDecOpen) {
        return m_neAACDecOpen();
    }
    return nullptr;
}

Configuration*
LibLoader::GetCurrentConfiguration(
        DecoderHandle hDecoder) const {
    if (m_neAACDecGetCurrentConfiguration) {
        return m_neAACDecGetCurrentConfiguration(hDecoder);
    }
    return nullptr;
}

unsigned char LibLoader::SetConfiguration(
        DecoderHandle hDecoder,
        Configuration* pConfig) const {
    if (m_neAACDecSetConfiguration) {
        return m_neAACDecSetConfiguration(hDecoder, pConfig);
    }
    // Return values:
    // 0 – Error, invalid configuration.
    // 1 – OK
    return 0;
}

long LibLoader::Init(
        DecoderHandle hDecoder,
        unsigned char* pBuffer,
        unsigned long sizeofBuffer,
        unsigned long* pSampleRate,
        unsigned char* pChannels) const {
    if (m_neAACDecInit) {
        return m_neAACDecInit(hDecoder,
                pBuffer,
                sizeofBuffer,
                pSampleRate,
                pChannels);
    }
    // Return values:
    // < 0 – Error
    // 0 - OK
    return -1;
}

char LibLoader::Init2(
        DecoderHandle hDecoder,
        unsigned char* pBuffer,
        unsigned long sizeofBuffer,
        unsigned long* pSampleRate,
        unsigned char* pChannels) const {
    if (m_neAACDecInit2) {
        return m_neAACDecInit2(hDecoder,
                pBuffer,
                sizeofBuffer,
                pSampleRate,
                pChannels);
    }
    // Return values:
    // < 0 – Error
    // 0 - OK
    return -1;
}

void LibLoader::Close(
        DecoderHandle hDecoder) const {
    if (m_neAACDecClose) {
        m_neAACDecClose(hDecoder);
    }
}

void LibLoader::PostSeekReset(
        DecoderHandle hDecoder,
        long frame) const {
    if (m_neAACDecPostSeekReset) {
        m_neAACDecPostSeekReset(hDecoder, frame);
    }
}

void* LibLoader::Decode2(
        DecoderHandle hDecoder,
        FrameInfo* pInfo,
        unsigned char* pBuffer,
        unsigned long bufferSize,
        void** ppSampleBuffer,
        unsigned long sampleBufferSize) const {
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

char* LibLoader::GetErrorMessage(
        unsigned char errcode) const {
    if (m_neAACDecGetErrorMessage) {
        return m_neAACDecGetErrorMessage(errcode);
    }
    return nullptr;
}

int LibLoader::GetVersion(
        char** faad_id_string,
        char** faad_copyright_string) const {
    if (m_neAACDecGetVersion) {
        return m_neAACDecGetVersion(faad_id_string, faad_copyright_string);
    }
    // Return values:
    // < 0 – Error
    // 0 - OK
    return -1;
}

} // namespace faad2

QDebug operator<<(
        QDebug dbg,
        const faad2::Configuration& cfg) {
    return dbg
            << "FAAD2 Configuration{"
            << "defObjectType:" << static_cast<int>(cfg.defObjectType)
            << "defSampleRate:" << cfg.defSampleRate
            << "outputFormat:" << static_cast<int>(cfg.outputFormat)
            << "downMatrix:" << static_cast<int>(cfg.downMatrix)
            << "useOldADTSFormat:" << static_cast<int>(cfg.useOldADTSFormat)
            << "dontUpSampleImplicitSBR:" << static_cast<int>(cfg.dontUpSampleImplicitSBR)
            << '}';
}
