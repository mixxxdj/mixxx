#include "sources/soundsourceproviderregistry.h"

#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("SoundSourceProviderRegistry");

} // anonymous namespace

void SoundSourceProviderRegistry::registerProvider(
        const SoundSourceProviderPointer& pProvider) {
    const QStringList supportedFileExtensions(
            pProvider->getSupportedFileExtensions());
    if (supportedFileExtensions.isEmpty()) {
        kLogger.warning() << "SoundSource provider"
                          << pProvider->getName()
                          << "does not support any file extensions";
        return; // abort registration
    }
    for (const auto& fileExt : supportedFileExtensions) {
        SoundSourceProviderPriority providerPriority(
                pProvider->getPriorityHint(fileExt));
        registerProviderForFileExtension(
                fileExt,
                pProvider,
                providerPriority);
    }
}

void SoundSourceProviderRegistry::registerProviderForFileExtension(
        const QString& fileExtension,
        const SoundSourceProviderPointer& pProvider,
        SoundSourceProviderPriority providerPriority) {
    SoundSourceProviderRegistration registration(pProvider, providerPriority);
    addRegistrationForFileExtension(fileExtension, std::move(registration));
}

void SoundSourceProviderRegistry::addRegistrationForFileExtension(
        const QString& fileExtension,
        SoundSourceProviderRegistration registration) {
    DEBUG_ASSERT(registration.getProvider());
    QList<SoundSourceProviderRegistration>& registrationsForFileExtension =
            m_registry[fileExtension];
    insertRegistration(&registrationsForFileExtension, registration);
}

void SoundSourceProviderRegistry::insertRegistration(
        QList<SoundSourceProviderRegistration>* pRegistrations,
        SoundSourceProviderRegistration registration) {
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

void SoundSourceProviderRegistry::deregisterProvider(
        const SoundSourceProviderPointer& pProvider) {
    const QStringList supportedFileExtensions(
            pProvider->getSupportedFileExtensions());
    for (const auto& fileExt : supportedFileExtensions) {
        deregisterProviderForFileExtension(fileExt, pProvider);
    }
}

void SoundSourceProviderRegistry::deregisterProviderForFileExtension(
        const QString& fileExtension,
        const SoundSourceProviderPointer& pProvider) {
    auto registryIter(m_registry.find(fileExtension));
    if (m_registry.end() != registryIter) {
        QList<SoundSourceProviderRegistration>& registrationsForFileExtension = registryIter.value();
        auto listIter = registrationsForFileExtension.begin();
        while (registrationsForFileExtension.end() != listIter) {
            if (listIter->getProvider() == pProvider) {
                listIter = registrationsForFileExtension.erase(listIter);
            } else {
                ++listIter;
            }
        }
        if (registrationsForFileExtension.isEmpty()) {
            m_registry.erase(registryIter);
        }
    }
}

QList<SoundSourceProviderRegistration>
SoundSourceProviderRegistry::getRegistrationsForFileExtension(
        const QString& fileExtension) const {
    auto i = m_registry.constFind(fileExtension);
    if (m_registry.constEnd() != i) {
        return i.value();
    } else {
        return QList<SoundSourceProviderRegistration>();
    }
}

} // namespace mixxx
