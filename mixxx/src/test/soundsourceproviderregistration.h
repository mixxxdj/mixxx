#pragma once
#include "sources/soundsourceproxy.h"

/// This class can be used as a mixin for or a member of test classes that
/// require accessing SoundSources.
class SoundSourceProviderRegistration {
  protected:
    SoundSourceProviderRegistration() {
        // SoundSourceProxy does not support tear-down, so we have to test to see if it's already
        // been run once.
        if (!SoundSourceProxy::isFileSuffixSupported("wav")) {
            const bool providersRegistered =
                    SoundSourceProxy::registerProviders();
            Q_UNUSED(providersRegistered);
            DEBUG_ASSERT(providersRegistered);
        }
    }
};
