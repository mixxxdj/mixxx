#include "sources/soundsourceproviderregistry.h"

#include "util/logger.h"
#include "util/make_const_iterator.h"

namespace mixxx {

namespace {

const Logger kLogger("SoundSourceProviderRegistry");

void insertRegistration(
        QList<SoundSourceProviderRegistration>* pRegistrations,
        SoundSourceProviderRegistration&& registration) {
    DEBUG_ASSERT(pRegistrations);
    auto listIter = pRegistrations->cbegin();
    // Perform a linear search through the list & insert
    while (pRegistrations->cend() != listIter) {
        // Priority comparison with <=: New registrations will be inserted
        // before existing registrations with equal priority, but after
        // existing registrations with higher priority.
        if (listIter->getProviderPriority() <= registration.getProviderPriority()) {
            listIter = constInsert(
                    pRegistrations,
                    listIter,
                    std::move(registration));
            DEBUG_ASSERT(pRegistrations->cend() != listIter);
            return;
        }
        ++listIter;
    }
    // List was empty or registration has the lowest priority
    pRegistrations->append(std::move(registration));
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
    const QStringList supportedFileTypes(
            pProvider->getSupportedFileTypes());
    if (supportedFileTypes.isEmpty()) {
        kLogger.warning()
                << "SoundSource provider"
                << displayName
                << "does not support any file extensions";
        return 0; // abort registration
    }
    for (const auto& fileType : supportedFileTypes) {
        const auto priority =
                pProvider->getPriorityHint(fileType);
        kLogger.debug()
                << "Registering file type"
                << fileType
                << "for provider"
                << displayName
                << "with priority"
                << priority;
        SoundSourceProviderRegistration registration(pProvider, priority);
        QList<SoundSourceProviderRegistration>& registrationsForFileType =
                m_registrationListsByFileType[fileType];
        insertRegistration(
                &registrationsForFileType,
                std::move(registration));
    }
    m_providersByDisplayName.insert(displayName, pProvider);
    DEBUG_ASSERT(m_providersByDisplayName.count(displayName) == 1);
    return supportedFileTypes.size();
}

QList<SoundSourceProviderRegistration>
SoundSourceProviderRegistry::getRegistrationsForFileType(
        const QString& fileType) const {
    auto i = m_registrationListsByFileType.constFind(fileType);
    if (m_registrationListsByFileType.constEnd() != i) {
        DEBUG_ASSERT(!i.value().isEmpty());
        return i.value();
    } else {
        kLogger.debug()
                << "No provider(s) registered for file extension"
                << fileType;
        return QList<SoundSourceProviderRegistration>();
    }
}

} // namespace mixxx
