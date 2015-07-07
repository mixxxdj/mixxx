#include "soundsourceproviderregistry.h"

namespace Mixxx {

/*static*/ const QList<SoundSourceProviderRegistration> SoundSourceProviderRegistry::EMPTY_REGISTRATION_LIST;

void SoundSourceProviderRegistry::registerProvider(
        const SoundSourceProviderPointer& pProvider) {
    const QStringList supportedFileExtensions(
            pProvider->getSupportedFileExtensions());
    if (supportedFileExtensions.isEmpty()) {
        qWarning() << "SoundSource provider"
                << pProvider->getName()
                << "does not support any file extensions";
        return; // abort registration
    }
    for (const auto& fileExt: supportedFileExtensions) {
        SoundSourceProviderPriority providerPriority(
                pProvider->getPriorityHint(fileExt));
        registerProviderForFileExtension(
                fileExt,
                pProvider,
                providerPriority);
    }
}

void SoundSourceProviderRegistry::registerPluginLibrary(
        const SoundSourcePluginLibraryPointer& pPluginLibrary) {
    SoundSourceProviderPointer pProvider(
            pPluginLibrary->createSoundSourceProvider());
    if (!pProvider) {
        qWarning() << "Failed to obtain SoundSource provider from plugin library"
                << pPluginLibrary->getFilePath();
        return; // abort registration
    }
    const QStringList supportedFileExtensions(
            pProvider->getSupportedFileExtensions());
    if (supportedFileExtensions.isEmpty()) {
        qWarning() << "SoundSource provider"
                << pProvider->getName()
                << "does not support any file extensions";
        return; // abort registration
    }
    for (const auto& fileExt: supportedFileExtensions) {
        SoundSourceProviderPriority providerPriority(
                pProvider->getPriorityHint(fileExt));
        registerPluginProviderForFileExtension(
                fileExt,
                pPluginLibrary,
                pProvider,
                providerPriority);
    }
}

void SoundSourceProviderRegistry::registerProviderForFileExtension(
        const QString& fileExtension,
        const SoundSourceProviderPointer& pProvider,
        SoundSourceProviderPriority providerPriority) {
    SoundSourceProviderRegistration registration(
        SoundSourcePluginLibraryPointer(), pProvider, providerPriority);
    addRegistrationForFileExtension(fileExtension, std::move(registration));
}

void SoundSourceProviderRegistry::registerPluginProviderForFileExtension(
        const QString& fileExtension,
        const SoundSourcePluginLibraryPointer& pPluginLibrary,
        const SoundSourceProviderPointer& pProvider,
        SoundSourceProviderPriority providerPriority) {
    SoundSourceProviderRegistration registration(
            pPluginLibrary, pProvider, providerPriority);
    addRegistrationForFileExtension(fileExtension, std::move(registration));
}

void SoundSourceProviderRegistry::addRegistrationForFileExtension(
        const QString& fileExtension,
        SoundSourceProviderRegistration registration) {
    DEBUG_ASSERT(registration.getProvider());
    QList<SoundSourceProviderRegistration>& registrationsForFileExtension =
            m_registry[fileExtension];
    QList<SoundSourceProviderRegistration>::iterator listIter(
            registrationsForFileExtension.begin());
    insertRegistration(&registrationsForFileExtension, registration);
}

void SoundSourceProviderRegistry::reregisterProviderForFileExtension(
        const QString& fileExtension,
        const SoundSourceProviderPointer& pProvider,
        SoundSourceProviderPriority providerPriority) {
    QList<SoundSourceProviderRegistration>& registrationsForFileExtension =
            m_registry[fileExtension];
    QList<SoundSourceProviderRegistration>::iterator listIter(
            registrationsForFileExtension.begin());
    // Perform a linear search through the list
    while (registrationsForFileExtension.end() != listIter) {
        if (listIter->getProvider() == pProvider) {
            if (listIter->getProviderPriority() != providerPriority) {
                SoundSourceProviderRegistration registration(*listIter);
                registrationsForFileExtension.erase(listIter);
                insertRegistration(&registrationsForFileExtension, std::move(registration));
            }
            // else nothing to do
            return; // done
        }
    }
}

void SoundSourceProviderRegistry::insertRegistration(
        QList<SoundSourceProviderRegistration> *pRegistrations,
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
    for (const auto& fileExt: supportedFileExtensions) {
        deregisterProviderForFileExtension(fileExt, pProvider);
    }
}

void SoundSourceProviderRegistry::deregisterProviderForFileExtension(
        const QString& fileExtension,
        const SoundSourceProviderPointer& pProvider) {
    auto mapIter(m_registry.find(fileExtension));
    if (m_registry.end() != mapIter) {
        QList<SoundSourceProviderRegistration>& registrationsForFileExtension = mapIter.value();
        auto listIter = registrationsForFileExtension.begin();
        while (registrationsForFileExtension.end() != listIter) {
            if (listIter->getProvider() == pProvider) {
                listIter = registrationsForFileExtension.erase(listIter);
            } else {
                ++listIter;
            }
        }
        if (registrationsForFileExtension.isEmpty()) {
            m_registry.erase(mapIter);
        }
    }
}

void SoundSourceProviderRegistry::deregisterPluginLibrary(
        const SoundSourcePluginLibraryPointer& pPluginLibrary) {
    auto mapIter(m_registry.begin());
    while (m_registry.end() != mapIter) {
        QList<SoundSourceProviderRegistration>& registrationsForFileExtension = mapIter.value();
        auto listIter = registrationsForFileExtension.begin();
        while (registrationsForFileExtension.end() != listIter) {
            if (listIter->getPluginLibrary() == pPluginLibrary) {
                listIter = registrationsForFileExtension.erase(listIter);
            } else {
                ++listIter;
            }
        }
        if (registrationsForFileExtension.isEmpty()) {
            mapIter = m_registry.erase(mapIter);
        } else {
            ++mapIter;
        }
    }
}

const QList<SoundSourceProviderRegistration>&
SoundSourceProviderRegistry::getRegistrationsForFileExtension(
        const QString& fileExtension) const {
    FileExtension2RegistrationList::const_iterator i(
            m_registry.find(fileExtension));
    if (m_registry.end() != i) {
        return i.value();
    } else {
        return EMPTY_REGISTRATION_LIST;
    }
}

} // Mixxx
