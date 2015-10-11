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
    addRegistrationForFileExtension(fileExtension, registration);
}

void SoundSourceProviderRegistry::registerPluginProviderForFileExtension(
        const QString& fileExtension,
        const SoundSourcePluginLibraryPointer& pPluginLibrary,
        const SoundSourceProviderPointer& pProvider,
        SoundSourceProviderPriority providerPriority) {
    SoundSourceProviderRegistration registration(
            pPluginLibrary, pProvider, providerPriority);
    addRegistrationForFileExtension(fileExtension, registration);
}

void SoundSourceProviderRegistry::addRegistrationForFileExtension(
        const QString& fileExtension,
        const SoundSourceProviderRegistration& registration) {
    DEBUG_ASSERT(registration.getProvider());
    QList<SoundSourceProviderRegistration>& registrationsForFileExtension =
            m_registrations[fileExtension];
    QList<SoundSourceProviderRegistration>::iterator i(
            registrationsForFileExtension.begin());
    // Perform a linear search through the list & insert
    while (registrationsForFileExtension.end() != i) {
        // Priority comparison with <=: New registrations will be inserted
        // before existing registrations with equal priority, but after
        // existing registrations with higher priority.
        if (i->getProviderPriority() <= registration.getProviderPriority()) {
            i = registrationsForFileExtension.insert(i, registration);
            DEBUG_ASSERT(registrationsForFileExtension.end() != i);
            break; // exit loop
        } else {
            ++i; // continue loop
        }
    }
    if (registrationsForFileExtension.end() == i) {
        // List was empty or registration has the lowest priority
        registrationsForFileExtension.append(registration);
    }
}

const QList<SoundSourceProviderRegistration>&
SoundSourceProviderRegistry::getRegistrationsForFileExtension(
        const QString& fileExtension) const {
    FileExtension2RegistrationList::const_iterator i(
            m_registrations.find(fileExtension));
    if (m_registrations.end() != i) {
        return i.value();
    } else {
        return EMPTY_REGISTRATION_LIST;
    }
}

} // Mixxx
