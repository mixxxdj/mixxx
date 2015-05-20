#include "soundsourcepluginlibrary.h"

#include <QMutexLocker>

namespace Mixxx {

/*static*/ QMutex SoundSourcePluginLibrary::s_loadedPluginLibrariesMutex;
/*static*/ QMap<QString, SoundSourcePluginLibraryPointer> SoundSourcePluginLibrary::s_loadedPluginLibraries;

/*static*/ SoundSourcePluginLibraryPointer SoundSourcePluginLibrary::load(
        const QString& libFilePath) {
    const QMutexLocker mutexLocker(&s_loadedPluginLibrariesMutex);

    if (s_loadedPluginLibraries.contains(libFilePath)) {
        return s_loadedPluginLibraries.value(libFilePath);
    } else {
        SoundSourcePluginLibraryPointer pPluginLibrary(
                new SoundSourcePluginLibrary(libFilePath));
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
      m_apiVersion(0),
      m_getSoundSourceProviderFunc(NULL) {
}

SoundSourcePluginLibrary::~SoundSourcePluginLibrary() {
}

bool SoundSourcePluginLibrary::init() {
    DEBUG_ASSERT(!m_library.isLoaded());
    if (!m_library.load()) {
        qWarning() << "Failed to dynamically load plugin library"
                << m_library.fileName()
                << ":" << m_library.errorString();
        return false;
    }
    qDebug() << "Dynamically loaded plugin library"
            << m_library.fileName();

    SoundSourcePluginAPI_getVersionFunc getVersionFunc = (SoundSourcePluginAPI_getVersionFunc)
            m_library.resolve(SoundSourcePluginAPI_getVersionFuncName);
    if (!getVersionFunc) {
        // Try to resolve the legacy plugin API function name
        getVersionFunc = (SoundSourcePluginAPI_getVersionFunc)
                    m_library.resolve("getSoundSourceAPIVersion");
    }
    if (getVersionFunc) {
        m_apiVersion = getVersionFunc();
        if (m_apiVersion == MIXXX_SOUNDSOURCEPLUGINAPI_VERSION) {
            m_getSoundSourceProviderFunc = (SoundSourcePluginAPI_getSoundSourceProviderFunc)
                    m_library.resolve(SoundSourcePluginAPI_getSoundSourceProviderFuncName);
            if (!m_getSoundSourceProviderFunc) {
                qWarning() << "Failed to resolve SoundSource plugin API function"
                        << SoundSourcePluginAPI_getSoundSourceProviderFuncName;
            }
        } else {
            qWarning() << "Incompatible SoundSource plugin API version"
                    << m_apiVersion << "<>" << MIXXX_SOUNDSOURCEPLUGINAPI_VERSION;
        }
    } else {
        qWarning() << "Failed to resolve SoundSource plugin API function"
                << SoundSourcePluginAPI_getVersionFuncName;
    }

    if (getVersionFunc && m_getSoundSourceProviderFunc) {
        return true;
    } else {
        qWarning() << "Incompatible SoundSource plugin"
                << m_library.fileName();
        return false;
    }
}

} // Mixxx
