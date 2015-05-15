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
    static SoundSourcePluginLibraryPointer load(const QString& fileName);

    virtual ~SoundSourcePluginLibrary();

    int getApiVersion() const {
        return m_apiVersion;
    }

    const QVector<QString>& getSupportedFileTypes() const {
        return m_supportedFileTypes;
    }

    SoundSourcePointer newSoundSource(const QUrl& url) const {
        DEBUG_ASSERT(m_newSoundSourceFunc);
        return (*m_newSoundSourceFunc)(url);
    }

protected:
    explicit SoundSourcePluginLibrary(const QString& fileName);

    virtual bool init();

private:
    static QMutex m_loadedPluginLibrariesMutex;
    static QMap<QString, Mixxx::SoundSourcePluginLibraryPointer> m_loadedPluginLibraries;

    QLibrary m_library;

    int m_apiVersion;
    QVector<QString> m_supportedFileTypes;

    SoundSourcePluginAPI_newSoundSourceFunc m_newSoundSourceFunc;
};

} // namespace Mixxx

#endif // MIXXX_SOUNDSOURCEPLUGINLIBRARY_H
