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
    /// Register a provider for all supported file types
    /// with their cooperative priority hint.
    ///
    /// Returns the number of registered file types.
    int registerProvider(
            const SoundSourceProviderPointer& pProvider);

    QStringList getRegisteredFileTypes() const {
        return m_registrationListsByFileType.keys();
    }

    /// Returns all registrations for the given file type.
    /// If no providers have been registered for this file type
    /// an empty list will be returned.
    QList<SoundSourceProviderRegistration> getRegistrationsForFileType(
            const QString& fileType) const;

    /// Returns the primary provider registration for the given file
    /// types if available.
    std::optional<SoundSourceProviderRegistration> getPrimaryRegistrationForFileType(
            const QString& fileType) const {
        const auto registrations =
                getRegistrationsForFileType(fileType);
        if (registrations.isEmpty()) {
            return std::nullopt;
        } else {
            return std::make_optional(registrations.first());
        }
    }

    /// Returns the primary provider for the given file
    /// types if available.
    SoundSourceProviderPointer getPrimaryProviderForFileType(
            const QString& fileType) const {
        const auto optProviderRegistration =
                getPrimaryRegistrationForFileType(fileType);
        if (optProviderRegistration) {
            return optProviderRegistration->getProvider();
        } else {
            return nullptr;
        }
    }

  private:
    typedef QMap<QString, QList<SoundSourceProviderRegistration>>
            RegistrationListByFileTypeMap;
    RegistrationListByFileTypeMap m_registrationListsByFileType;

    typedef QMap<QString, SoundSourceProviderPointer> ProviderByDisplayNameMap;
    ProviderByDisplayNameMap m_providersByDisplayName;
};

} // namespace mixxx
