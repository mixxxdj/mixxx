#include "soundsourcepluginlibrary.h"

#include "util/logger.h"

#include <QMutexLocker>

namespace mixxx {

namespace {

const Logger kLogger("SoundSourcePluginLibrary");

} // anonymous namespace

/*static*/ QMutex SoundSourcePluginLibrary::s_loadedPluginLibrariesMutex;
/*static*/ QMap<QString, SoundSourcePluginLibraryPointer> SoundSourcePluginLibrary::s_loadedPluginLibraries;

/*static*/ SoundSourcePluginLibraryPointer SoundSourcePluginLibrary::load(
        const QString& libFilePath) {
    const QMutexLocker mutexLocker(&s_loadedPluginLibrariesMutex);

    if (s_loadedPluginLibraries.contains(libFilePath)) {
        return s_loadedPluginLibraries.value(libFilePath);
    } else {
        auto pPluginLibrary =
                std::make_shared<SoundSourcePluginLibrary>(libFilePath);
        if (pPluginLibrary->init()) {
            s_loadedPluginLibraries.insert(libFilePath, pPluginLibrary);
            return pPluginLibrary;
        } else {
            return SoundSourcePluginLibraryPointer();
        }
    }
}

SoundSourcePluginLibrary::SoundSourcePluginLibrary(const QString& libFilePath)
    : m_library(libFilePath),
      m_apiVersion(0) {
}

SoundSourcePluginLibrary::~SoundSourcePluginLibrary() {
}

bool SoundSourcePluginLibrary::init() {
    DEBUG_ASSERT(!m_library.isLoaded());
    if (!m_library.load()) {
        kLogger.warning() << "Failed to dynamically load plugin library"
                << m_library.fileName()
                << ":" << m_library.errorString();
        return false;
    }
    kLogger.debug() << "Dynamically loaded plugin library"
            << m_library.fileName();

    SoundSourcePluginAPI_getVersionFunc getVersionFunc = (SoundSourcePluginAPI_getVersionFunc)
            m_library.resolve(SoundSourcePluginAPI_getVersionFuncName);
    if (getVersionFunc == nullptr) {
        // Try to resolve the legacy plugin API function name
        getVersionFunc = (SoundSourcePluginAPI_getVersionFunc)
                    m_library.resolve("getSoundSourceAPIVersion");
    }
    if (getVersionFunc == nullptr) {
        kLogger.warning() << "Failed to resolve SoundSource plugin API function"
                << SoundSourcePluginAPI_getVersionFuncName;
        return initFailedForIncompatiblePlugin();
    }

    m_apiVersion = getVersionFunc();
    if (m_apiVersion != MIXXX_SOUNDSOURCEPLUGINAPI_VERSION) {
        kLogger.warning() << "Incompatible SoundSource plugin API version"
                << m_apiVersion << "<>" << MIXXX_SOUNDSOURCEPLUGINAPI_VERSION;
        return initFailedForIncompatiblePlugin();
    }

    auto const createSoundSourceProviderFunc =
            reinterpret_cast<SoundSourcePluginAPI_createSoundSourceProviderFunc>(
                    m_library.resolve(SoundSourcePluginAPI_createSoundSourceProviderFuncName));
    if (createSoundSourceProviderFunc == nullptr) {
        kLogger.warning() << "Failed to resolve SoundSource plugin API function"
                << SoundSourcePluginAPI_createSoundSourceProviderFuncName;
        return initFailedForIncompatiblePlugin();
    }

    auto const destroySoundSourceProviderFunc =
            reinterpret_cast<SoundSourcePluginAPI_destroySoundSourceProviderFunc>(
                m_library.resolve(SoundSourcePluginAPI_destroySoundSourceProviderFuncName));
    if (destroySoundSourceProviderFunc == nullptr) {
        kLogger.warning() << "Failed to resolve SoundSource plugin API function"
                << SoundSourcePluginAPI_destroySoundSourceProviderFuncName;
        return initFailedForIncompatiblePlugin();
    }

    m_pSoundSourceProvider = SoundSourceProviderPointer(
            (*createSoundSourceProviderFunc)(),
            destroySoundSourceProviderFunc);
    if (m_pSoundSourceProvider) {
        return true;
    } else {
        kLogger.warning() << "Failed to create SoundSource provider for plugin library"
                << m_library.fileName();
        return false;
    }
}

bool SoundSourcePluginLibrary::initFailedForIncompatiblePlugin() const {
    kLogger.warning() << "Incompatible SoundSource plugin"
            << m_library.fileName();
    return false;
}

SoundSourceProviderPointer SoundSourcePluginLibrary::getSoundSourceProvider() const {
    return m_pSoundSourceProvider;
}

} // Mixxx
