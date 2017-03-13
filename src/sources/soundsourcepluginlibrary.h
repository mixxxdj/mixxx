#ifndef MIXXX_SOUNDSOURCEPLUGINLIBRARY_H
#define MIXXX_SOUNDSOURCEPLUGINLIBRARY_H

#include "sources/soundsourcepluginapi.h"

#include <QMap>
#include <QMutex>
#include <QLibrary>

namespace Mixxx {

class SoundSourcePluginLibrary;

typedef QSharedPointer<SoundSourcePluginLibrary> SoundSourcePluginLibraryPointer;

// Wrapper class for a dynamic library that implements the SoundSource plugin API
class SoundSourcePluginLibrary {
public:
    static SoundSourcePluginLibraryPointer load(const QString& libFilePath);

    virtual ~SoundSourcePluginLibrary();

    QString getFilePath() const {
        return m_library.fileName();
    }

    int getApiVersion() const {
        return m_apiVersion;
    }

    SoundSourceProviderPointer getSoundSourceProvider() {
        DEBUG_ASSERT(m_getSoundSourceProviderFunc);
        return (*m_getSoundSourceProviderFunc)();
    }

protected:
    explicit SoundSourcePluginLibrary(const QString& libFilePath);

    virtual bool init();

private:
    static QMutex s_loadedPluginLibrariesMutex;
    static QMap<QString, Mixxx::SoundSourcePluginLibraryPointer> s_loadedPluginLibraries;

    QLibrary m_library;

    int m_apiVersion;
    QStringList m_supportedFileExtensions;

    SoundSourcePluginAPI_getSoundSourceProviderFunc m_getSoundSourceProviderFunc;
};

} // namespace Mixxx

#endif // MIXXX_SOUNDSOURCEPLUGINLIBRARY_H
