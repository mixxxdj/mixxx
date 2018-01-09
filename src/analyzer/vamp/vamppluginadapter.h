#pragma once

#include <vamp-hostsdk/vamp-hostsdk.h>

#include "util/assert.h"
#include "util/memory.h"


namespace mixxx {

// Thread-safe adapter/decorator for the partially thread-unsafe
// PluginLoader/Plugin couple
class VampPluginAdapter {
  public:
    static Vamp::HostExt::PluginLoader::PluginKeyList listPlugins();

    static Vamp::HostExt::PluginLoader::PluginKey composePluginKey(
            std::string libraryName,
            std::string identifier);

    VampPluginAdapter() = default;
    VampPluginAdapter(
            Vamp::HostExt::PluginLoader::PluginKey key,
            float inputSampleRate,
            int adapterFlags = 0);
    VampPluginAdapter(VampPluginAdapter&&) = default;
    virtual ~VampPluginAdapter();

    VampPluginAdapter& operator=(VampPluginAdapter&&) = delete;
    VampPluginAdapter& operator=(const VampPluginAdapter&) = delete;

    void loadPlugin(
            Vamp::HostExt::PluginLoader::PluginKey key,
            float inputSampleRate,
            int adapterFlags = 0);

    operator bool() const {
        return m_plugin != nullptr;
    }

    Vamp::Plugin* operator->() {
        DEBUG_ASSERT(m_plugin);
        return m_plugin.get();
    }
    const Vamp::Plugin* operator->() const {
        DEBUG_ASSERT(m_plugin);
        return m_plugin.get();
    }

    Vamp::Plugin& operator*() {
        DEBUG_ASSERT(m_plugin);
        return *m_plugin.get();
    }
    const Vamp::Plugin& operator*() const {
        DEBUG_ASSERT(m_plugin);
        return *m_plugin.get();
    }

  private:
    // This pointer must never be deleted implicitly to avoid race conditions!
    // It is only needed for utilizing the default move constructor.
    std::unique_ptr<Vamp::Plugin> m_plugin;
};

} // namespace mixxx
