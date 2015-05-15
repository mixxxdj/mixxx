#include "soundsourcepluginlibrary.h"

#include <QMutexLocker>

namespace Mixxx {

/*static*/ QMutex SoundSourcePluginLibrary::m_loadedPluginLibrariesMutex;
/*static*/ QMap<QString, SoundSourcePluginLibraryPointer> SoundSourcePluginLibrary::m_loadedPluginLibraries;

/*static*/ SoundSourcePluginLibraryPointer SoundSourcePluginLibrary::load(
        const QString& fileName) {
    const QMutexLocker mutexLocker(&m_loadedPluginLibrariesMutex);

    if (m_loadedPluginLibraries.contains(fileName)) {
        return m_loadedPluginLibraries.value(fileName);
    } else {
        SoundSourcePluginLibraryPointer pPluginLibrary(
                new SoundSourcePluginLibrary(fileName));
        if (pPluginLibrary->init()) {
            m_loadedPluginLibraries.insert(fileName, pPluginLibrary);
            return pPluginLibrary;
        } else {
            return SoundSourcePluginLibraryPointer();
        }
    }
}

SoundSourcePluginLibrary::SoundSourcePluginLibrary(const QString& fileName)
    : m_library(fileName),
      m_apiVersion(0),
      m_newSoundSourceFunc(NULL) {
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
    SoundSourcePluginAPI_getSupportedFileTypesFunc getSupportedFileTypesFunc = NULL;
    if (getVersionFunc) {
        m_apiVersion = getVersionFunc();
        if (m_apiVersion == MIXXX_SOUNDSOURCEPLUGINAPI_VERSION) {
            getSupportedFileTypesFunc = (SoundSourcePluginAPI_getSupportedFileTypesFunc)
                    m_library.resolve(SoundSourcePluginAPI_getSupportedFileTypesFuncName);
            if (!getSupportedFileTypesFunc) {
                qWarning() << "Failed to resolve SoundSource plugin API function"
                        << SoundSourcePluginAPI_getSupportedFileTypesFuncName;
            }
            m_newSoundSourceFunc = (SoundSourcePluginAPI_newSoundSourceFunc)
                    m_library.resolve(SoundSourcePluginAPI_newSoundSourceFuncName);
            if (!m_newSoundSourceFunc) {
                qWarning() << "Failed to resolve SoundSource plugin API function"
                        << SoundSourcePluginAPI_newSoundSourceFuncName;
            }
        } else {
            qWarning() << "Incompatible SoundSource plugin API version"
                    << m_apiVersion << "<>" << MIXXX_SOUNDSOURCEPLUGINAPI_VERSION;
        }
    } else {
        qWarning() << "Failed to resolve SoundSource plugin API function"
                << SoundSourcePluginAPI_getVersionFuncName;
    }

    if (getVersionFunc && getSupportedFileTypesFunc && m_newSoundSourceFunc) {
        m_supportedFileTypes = (*getSupportedFileTypesFunc)();
        if (m_supportedFileTypes.isEmpty()) {
            qWarning() << "SoundSource plugin does not support any file types"
                    << m_library.fileName();
            return false;
        } else {
            return true;
        }
    } else {
        qWarning() << "Incompatible SoundSource plugin"
                << m_library.fileName();
        return false;
    }
}

} // Mixxx
