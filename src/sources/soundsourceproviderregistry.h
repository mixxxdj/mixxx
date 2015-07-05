#ifndef MIXXX_SOUNDSOURCEPROVIDERREGISTRY_H
#define MIXXX_SOUNDSOURCEPROVIDERREGISTRY_H

#include "sources/soundsourcepluginlibrary.h"

#include <QMap>

namespace Mixxx {

class SoundSourceProviderRegistration {
public:
    const SoundSourcePluginLibraryPointer& getPluginLibrary() const {
        return m_pPluginLibrary;
    }
    const SoundSourceProviderPointer& getProvider() const {
        return m_pProvider;
    }
    SoundSourceProviderPriority getProviderPriority() const {
        return m_providerPriority;
    }

private:
    friend class SoundSourceProviderRegistry;
    SoundSourceProviderRegistration(
            SoundSourcePluginLibraryPointer pPluginLibrary,
            SoundSourceProviderPointer pProvider,
            SoundSourceProviderPriority providerPriority)
        : m_pPluginLibrary(pPluginLibrary),
          m_pProvider(pProvider),
          m_providerPriority(providerPriority) {
    }

    SoundSourcePluginLibraryPointer m_pPluginLibrary;
    SoundSourceProviderPointer m_pProvider;
    SoundSourceProviderPriority m_providerPriority;
};

// Registry for SoundSourceProviders
class SoundSourceProviderRegistry {
public:
    void registerProvider(
            const SoundSourceProviderPointer& pProvider);
    void registerProvider(
            const SoundSourceProviderPointer& pProvider,
            SoundSourceProviderPriority priority);
    void registerPluginLibrary(
            const SoundSourcePluginLibraryPointer& pPluginLibrary);
    void registerPluginLibrary(
            const SoundSourcePluginLibraryPointer& pPluginLibrary,
            SoundSourceProviderPriority priority);

    QStringList getRegisteredFileExtensions() const {
        return m_registrations.keys();
    }

    const QList<SoundSourceProviderRegistration>& getRegistrationsForFileExtension(const QString& fileExtension) const;

private:
    static const QList<SoundSourceProviderRegistration> EMPTY_REGISTRATION_LIST;

    typedef QMap<QString, QList<SoundSourceProviderRegistration>> FileExtension2RegistrationList;

    void addRegistration(const SoundSourceProviderRegistration& registration);

    FileExtension2RegistrationList m_registrations;
};

} // namespace Mixxx

#endif // MIXXX_SOUNDSOURCEPROVIDERREGISTRY_H
