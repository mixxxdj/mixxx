#ifndef MIXXX_SOUNDSOURCEPLUGINLIBRARY_H
#define MIXXX_SOUNDSOURCEPLUGINLIBRARY_H

#include "sources/soundsourcepluginapi.h"

#include <QMap>
#include <QMutex>
#include <QLibrary>

namespace mixxx {

class SoundSourcePluginLibrary;

typedef QSharedPointer<SoundSourcePluginLibrary> SoundSourcePluginLibraryPointer;

typedef QSharedPointer<SoundSourceProvider> SoundSourceProviderPointer;

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

    SoundSourceProviderPointer createSoundSourceProvider() const;

protected:
    explicit SoundSourcePluginLibrary(const QString& libFilePath);

    virtual bool init();

private:
    static QMutex s_loadedPluginLibrariesMutex;
    static QMap<QString, mixxx::SoundSourcePluginLibraryPointer> s_loadedPluginLibraries;

    QLibrary m_library;

    int m_apiVersion;
    QStringList m_supportedFileExtensions;

    SoundSourcePluginAPI_createSoundSourceProviderFunc m_createSoundSourceProviderFunc;
    SoundSourcePluginAPI_destroySoundSourceProviderFunc m_destroySoundSourceProviderFunc;
};

} // namespace mixxx

#endif // MIXXX_SOUNDSOURCEPLUGINLIBRARY_H
