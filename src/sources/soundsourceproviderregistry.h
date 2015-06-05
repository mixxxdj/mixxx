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
    SoundSourceProvider::Priority getProviderPriority() const {
        return m_providerPriority;
    }

private:
    friend class SoundSourceProviderRegistry;
    SoundSourceProviderRegistration(
            SoundSourcePluginLibraryPointer pPluginLibrary,
            SoundSourceProviderPointer pProvider,
            SoundSourceProvider::Priority providerPriority)
        : m_pPluginLibrary(pPluginLibrary),
          m_pProvider(pProvider),
          m_providerPriority(providerPriority) {
    }

    SoundSourcePluginLibraryPointer m_pPluginLibrary;
    SoundSourceProviderPointer m_pProvider;
    SoundSourceProvider::Priority m_providerPriority;
};

typedef QList<SoundSourceProviderRegistration> SoundSourceProviderRegistrationList;

// Registry for SoundSourceProviders
class SoundSourceProviderRegistry {
public:
    void registerProvider(
            const SoundSourceProviderPointer& pProvider);
    void registerProvider(
            const SoundSourceProviderPointer& pProvider,
            SoundSourceProvider::Priority priority);
    void registerPluginLibrary(
            const SoundSourcePluginLibraryPointer& pPluginLibrary);
    void registerPluginLibrary(
            const SoundSourcePluginLibraryPointer& pPluginLibrary,
            SoundSourceProvider::Priority priority);

    QStringList getRegisteredFileExtensions() const {
        return m_registrations.keys();
    }

    const SoundSourceProviderRegistrationList& getRegistrationsForFileExtension(const QString& fileExtension) const;

private:
    static const SoundSourceProviderRegistrationList EMPTY_REGISTRATION_LIST;

    typedef QMap<QString, SoundSourceProviderRegistrationList> FileExtension2RegistrationList;

    void addRegistration(const SoundSourceProviderRegistration& registration);

    FileExtension2RegistrationList m_registrations;
};

} // namespace Mixxx

#endif // MIXXX_SOUNDSOURCEPROVIDERREGISTRY_H
