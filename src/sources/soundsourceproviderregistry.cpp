#include "sources/soundsourceproviderregistry.h"

#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("SoundSourceProviderRegistry");

void insertRegistration(
        QList<SoundSourceProviderRegistration>* pRegistrations,
        SoundSourceProviderRegistration&& registration) {
    DEBUG_ASSERT(pRegistrations);
    QList<SoundSourceProviderRegistration>::iterator listIter(
            pRegistrations->begin());
    // Perform a linear search through the list & insert
    while (pRegistrations->end() != listIter) {
        // Priority comparison with <=: New registrations will be inserted
        // before existing registrations with equal priority, but after
        // existing registrations with higher priority.
        if (listIter->getProviderPriority() <= registration.getProviderPriority()) {
            listIter = pRegistrations->insert(listIter, std::move(registration));
            DEBUG_ASSERT(pRegistrations->end() != listIter);
            return; // done
        } else {
            ++listIter; // continue loop
        }
    }
    if (pRegistrations->end() == listIter) {
        // List was empty or registration has the lowest priority
        pRegistrations->append(std::move(registration));
    }
}

} // anonymous namespace

int SoundSourceProviderRegistry::registerProvider(
        const SoundSourceProviderPointer& pProvider) {
    VERIFY_OR_DEBUG_ASSERT(pProvider) {
        return 0;
    }
    const QString displayName = pProvider->getDisplayName();
    VERIFY_OR_DEBUG_ASSERT(!displayName.isEmpty()) {
        return 0;
    }
    kLogger.debug()
            << "Registering provider"
            << displayName;
    {
        // Due to the debug assertion this code is not testable
        const auto pProviderByDisplayName = m_providersByDisplayName.value(displayName);
        VERIFY_OR_DEBUG_ASSERT(!pProviderByDisplayName) {
            if (pProviderByDisplayName == pProvider) {
                kLogger.info()
                        << "Ignoring repeated registration of the same provider"
                        << displayName;
            } else {
                kLogger.warning()
                        << "Cannot register different providers with the same display name"
                        << displayName;
            }
            return 0;
        }
    }
    const QStringList supportedFileExtensions(
            pProvider->getSupportedFileExtensions());
    if (supportedFileExtensions.isEmpty()) {
        kLogger.warning()
                << "SoundSource provider"
                << displayName
                << "does not support any file extensions";
        return 0; // abort registration
    }
    for (const auto& fileExtension : supportedFileExtensions) {
        const auto priority =
                pProvider->getPriorityHint(fileExtension);
        kLogger.debug()
                << "Registering file extension"
                << fileExtension
                << "for provider"
                << displayName
                << "with priority"
                << priority;
        SoundSourceProviderRegistration registration(pProvider, priority);
        QList<SoundSourceProviderRegistration>& registrationsForFileExtension =
                m_registrationListsByFileExtension[fileExtension];
        insertRegistration(
                &registrationsForFileExtension,
                std::move(registration));
    }
    m_providersByDisplayName.insert(displayName, pProvider);
    DEBUG_ASSERT(m_providersByDisplayName.count(displayName) == 1);
    return supportedFileExtensions.size();
}

QList<SoundSourceProviderRegistration>
SoundSourceProviderRegistry::getRegistrationsForFileExtension(
        const QString& fileExtension) const {
    auto i = m_registrationListsByFileExtension.constFind(fileExtension);
    if (m_registrationListsByFileExtension.constEnd() != i) {
        DEBUG_ASSERT(!i.value().isEmpty());
        return i.value();
    } else {
        kLogger.debug()
                << "No provider(s) registered for file extension"
                << fileExtension;
        return QList<SoundSourceProviderRegistration>();
    }
}

} // namespace mixxx
