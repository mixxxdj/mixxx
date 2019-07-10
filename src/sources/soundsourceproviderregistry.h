#ifndef MIXXX_SOUNDSOURCEPROVIDERREGISTRY_H
#define MIXXX_SOUNDSOURCEPROVIDERREGISTRY_H

#include <QMap>

#include "sources/soundsourceprovider.h"

namespace mixxx {

class SoundSourceProviderRegistration {
  public:
    const SoundSourceProviderPointer& getProvider() const {
        return m_pProvider;
    }
    SoundSourceProviderPriority getProviderPriority() const {
        return m_providerPriority;
    }

  private:
    friend class SoundSourceProviderRegistry;
    SoundSourceProviderRegistration(
            SoundSourceProviderPointer pProvider,
            SoundSourceProviderPriority providerPriority)
            : m_pProvider(pProvider),
              m_providerPriority(providerPriority) {
    }

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

    // Registers a provider for a single file extension with an
    // explicitly specified priority. The provider must support
    // the given file extension.
    void registerProviderForFileExtension(
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

} // namespace mixxx

#endif // MIXXX_SOUNDSOURCEPROVIDERREGISTRY_H
