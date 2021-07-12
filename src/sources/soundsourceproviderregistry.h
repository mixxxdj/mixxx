#pragma once

#include <QMap>

#include "sources/soundsourceprovider.h"
#include "util/optional.h"

namespace mixxx {

class SoundSourceProviderRegistration final {
  public:
    SoundSourceProviderRegistration(SoundSourceProviderRegistration&&) = default;
    SoundSourceProviderRegistration(const SoundSourceProviderRegistration&) = default;
    SoundSourceProviderRegistration& operator=(SoundSourceProviderRegistration&&) = default;
    SoundSourceProviderRegistration& operator=(const SoundSourceProviderRegistration&) = default;

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
            : m_pProvider(std::move(pProvider)),
              m_providerPriority(providerPriority) {
        DEBUG_ASSERT(m_pProvider);
    }

    SoundSourceProviderPointer m_pProvider;
    SoundSourceProviderPriority m_providerPriority;
};

/// Registry for SoundSourceProviders
class SoundSourceProviderRegistry {
  public:
    /// Register a provider for all supported file extensions
    /// with their cooperative priority hint.
    ///
    /// Returns the number of registered file extensions.
    int registerProvider(
            const SoundSourceProviderPointer& pProvider);

    QStringList getRegisteredFileExtensions() const {
        return m_registrationListsByFileExtension.keys();
    }

    /// Returns all registrations for the given file extension.
    /// If no providers have been registered for this file extension
    /// an empty list will be returned.
    QList<SoundSourceProviderRegistration> getRegistrationsForFileExtension(
            const QString& fileExtension) const;

    /// Returns the primary provider registration for the given file
    /// extensions if available.
    std::optional<SoundSourceProviderRegistration> getPrimaryRegistrationForFileExtension(
            const QString& fileExtension) const {
        const auto registrations =
                getRegistrationsForFileExtension(fileExtension);
        if (registrations.isEmpty()) {
            return std::nullopt;
        } else {
            return std::make_optional(registrations.first());
        }
    }

    /// Returns the primary provider for the given file
    /// extensions if available.
    SoundSourceProviderPointer getPrimaryProviderForFileExtension(
            const QString& fileExtension) const {
        const auto optProviderRegistration =
                getPrimaryRegistrationForFileExtension(fileExtension);
        if (optProviderRegistration) {
            return optProviderRegistration->getProvider();
        } else {
            return nullptr;
        }
    }

  private:
    typedef QMap<QString, QList<SoundSourceProviderRegistration>>
            RegistrationListByFileExtensionMap;
    RegistrationListByFileExtensionMap m_registrationListsByFileExtension;

    typedef QMap<QString, SoundSourceProviderPointer> ProviderByDisplayNameMap;
    ProviderByDisplayNameMap m_providersByDisplayName;
};

} // namespace mixxx
