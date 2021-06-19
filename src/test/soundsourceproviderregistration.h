#pragma once
#include "sources/soundsourceproxy.h"

/// This class can be used as a mixin for or a member of test classes that
/// require accessing SoundSources.
class SoundSourceProviderRegistration {
  protected:
    SoundSourceProviderRegistration() {
        const bool providersRegistered =
                SoundSourceProxy::registerProviders();
        Q_UNUSED(providersRegistered);
        DEBUG_ASSERT(providersRegistered);
    }
};
