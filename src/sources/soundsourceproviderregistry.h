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
    // Registers a provider for all supported file extensions
    // with their cooperative priority hint.
    void registerProvider(
            const SoundSourceProviderPointer& pProvider);
    // Registers a provider from a plugin library for all supported
    // file extensions with their cooperative priority hint.
    void registerPluginLibrary(
            const SoundSourcePluginLibraryPointer& pPluginLibrary);

    // Registers a provider for a single file extension with an
    // explicitly specified priority. The provider must support
    // the given file extension.
    void registerProviderForFileExtension(
            const QString& fileExtension,
            const SoundSourceProviderPointer& pProvider,
            SoundSourceProviderPriority providerPriority);
    // Registers a provider from a plugin library for a single file
    // extension with an explicitly specified priority. The provider
    // must support the given file extension.
    void registerPluginProviderForFileExtension(
            const QString& fileExtension,
            const SoundSourcePluginLibraryPointer& pPluginLibrary,
            const SoundSourceProviderPointer& pProvider,
            SoundSourceProviderPriority providerPriority);

    // Deregisters a provider for all supported file extensions.
    void deregisterProvider(
            const SoundSourceProviderPointer& pProvider);
    // Deregisters a provider for a single file extension.
    void deregisterProviderForFileExtension(
            const QString& fileExtension,
            const SoundSourceProviderPointer& pProvider);

    // Deregisters all providers from a plugin library.
    void deregisterPluginLibrary(
            const SoundSourcePluginLibraryPointer& pPluginLibrary);

    QStringList getRegisteredFileExtensions() const {
        return m_registry.keys();
    }

    // Returns all registrations for the given file extension.
    // If no providers have been registered for this file extension
    // an empty list will be returned.
    QList<SoundSourceProviderRegistration> getRegistrationsForFileExtension(
            const QString& fileExtension) const;

private:
    void addRegistrationForFileExtension(
            const QString& fileExtension,
            SoundSourceProviderRegistration registration);

    static void insertRegistration(
            QList<SoundSourceProviderRegistration>* pRegistrations,
            SoundSourceProviderRegistration registration);

    typedef QMap<QString, QList<SoundSourceProviderRegistration>> FileExtension2RegistrationList;

    FileExtension2RegistrationList m_registry;
};

} // namespace Mixxx

#endif // MIXXX_SOUNDSOURCEPROVIDERREGISTRY_H
