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
    // with their implicit priority hint.
    void registerProvider(
            const SoundSourceProviderPointer& pProvider);
    // Registers a provider from a plugin library for all supported
    // file extensions with their implicit priority hint.
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

    // Reregister a provider that is already registered with
    // a new priority. It doesn't matter if the provider is
    // built-in or provided by a plugin library.
    void reregisterProviderForFileExtension(
            const QString& fileExtension,
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
        return m_registrations.keys();
    }

    // Returns all registrations for the given file extension.
    // If no providers have been registered for this file extension
    // an empty list will be returned.
    const QList<SoundSourceProviderRegistration>& getRegistrationsForFileExtension(
            const QString& fileExtension) const;

private:
    static const QList<SoundSourceProviderRegistration> EMPTY_REGISTRATION_LIST;

    typedef QMap<QString, QList<SoundSourceProviderRegistration>> FileExtension2RegistrationList;

    void addRegistrationForFileExtension(
            const QString& fileExtension,
            SoundSourceProviderRegistration registration);

    static void insertRegistration(
            QList<SoundSourceProviderRegistration>* pRegistrations,
            SoundSourceProviderRegistration registration);

    FileExtension2RegistrationList m_registrations;
};

} // namespace Mixxx

#endif // MIXXX_SOUNDSOURCEPROVIDERREGISTRY_H
