#include "soundsourceproviderregistry.h"

namespace Mixxx {

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
    insertRegistration(&registrationsForFileExtension, registration);
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

void SoundSourceProviderRegistry::deregisterPluginLibrary(
        const SoundSourcePluginLibraryPointer& pPluginLibrary) {
    auto registryIter(m_registry.begin());
    while (m_registry.end() != registryIter) {
        QList<SoundSourceProviderRegistration>& registrationsForFileExtension = registryIter.value();
        auto listIter = registrationsForFileExtension.begin();
        while (registrationsForFileExtension.end() != listIter) {
            if (listIter->getPluginLibrary() == pPluginLibrary) {
                listIter = registrationsForFileExtension.erase(listIter);
            } else {
                ++listIter;
            }
        }
        if (registrationsForFileExtension.isEmpty()) {
            registryIter = m_registry.erase(registryIter);
        } else {
            ++registryIter;
        }
    }
}

QList<SoundSourceProviderRegistration>
SoundSourceProviderRegistry::getRegistrationsForFileExtension(
        const QString& fileExtension) const {
    FileExtension2RegistrationList::const_iterator i(
            m_registry.find(fileExtension));
    if (m_registry.end() != i) {
        return i.value();
    } else {
        return QList<SoundSourceProviderRegistration>();
    }
}

} // Mixxx
