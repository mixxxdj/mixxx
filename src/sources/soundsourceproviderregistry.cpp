#include "soundsourceproviderregistry.h"

namespace Mixxx {

/*static*/ const SoundSourceProviderRegistrationList SoundSourceProviderRegistry::EMPTY_REGISTRATION_LIST;

void SoundSourceProviderRegistry::registerProvider(
        const SoundSourceProviderPointer& pProvider) {
    registerProvider(pProvider, pProvider->getPriorityHint());
}

void SoundSourceProviderRegistry::registerProvider(
        const SoundSourceProviderPointer& pProvider,
        SoundSourceProvider::Priority providerPriority) {
    SoundSourceProviderRegistration registration(
            SoundSourcePluginLibraryPointer(), pProvider, providerPriority);
    addRegistration(registration);
}

void SoundSourceProviderRegistry::registerPluginLibrary(
        const SoundSourcePluginLibraryPointer& pPluginLibrary) {
    SoundSourceProviderPointer pProvider(
            pPluginLibrary->createSoundSourceProvider());
    SoundSourceProviderRegistration registration(
            pPluginLibrary, pProvider, pProvider->getPriorityHint());
    addRegistration(registration);
}

void SoundSourceProviderRegistry::registerPluginLibrary(
        const SoundSourcePluginLibraryPointer& pPluginLibrary,
        SoundSourceProvider::Priority providerPriority) {
    SoundSourceProviderPointer pProvider(
            pPluginLibrary->getSoundSourceProvider());
    SoundSourceProviderRegistration registration(
            pPluginLibrary, pProvider, providerPriority);
    addRegistration(registration);
}

void SoundSourceProviderRegistry::addRegistration(const SoundSourceProviderRegistration& registration) {
    DEBUG_ASSERT(registration.getProvider());
    const QStringList supportedFileExtensions(
            registration.getProvider()->getSupportedFileExtensions());
    if (supportedFileExtensions.isEmpty()) {
        qWarning() << "SoundSource provider"
                << registration.getProvider()->getName()
                << "does not support any file types - aborting registration!";
        return; // abort registration
    }
    for (const auto& supportedFileExtension: supportedFileExtensions) {
        SoundSourceProviderRegistrationList& registrationsForFileExtension =
                m_registrations[supportedFileExtension];
        SoundSourceProviderRegistrationList::iterator i(registrationsForFileExtension.begin());
        // Linear search through the list & insert
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
}

const SoundSourceProviderRegistrationList&
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
