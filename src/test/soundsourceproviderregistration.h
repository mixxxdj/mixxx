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
            [[maybe_unused]] const bool providersRegistered =
                    SoundSourceProxy::registerProviders();
            DEBUG_ASSERT(providersRegistered);
        }
    }
};
