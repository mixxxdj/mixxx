#pragma once

#include <vamp-hostsdk/vamp-hostsdk.h>

#include "util/assert.h"
#include "util/memory.h"


namespace mixxx {

// Thread-safe adapter/decorator for the thread-unsafe Vamp plugin C++ API
class VampPluginAdapter {
  public:
    static Vamp::HostExt::PluginLoader::PluginKeyList listPlugins();

    static Vamp::HostExt::PluginLoader::PluginKey composePluginKey(
            std::string libraryName,
            std::string identifier);

    VampPluginAdapter();
    VampPluginAdapter(
            Vamp::HostExt::PluginLoader::PluginKey key,
            float inputSampleRate,
            int adapterFlags = 0);
    VampPluginAdapter(VampPluginAdapter&&) = default;
    virtual ~VampPluginAdapter();

    VampPluginAdapter& operator=(VampPluginAdapter&&) = delete;
    VampPluginAdapter& operator=(const VampPluginAdapter&) = delete;

    operator bool() const {
        return m_plugin != nullptr;
    }

    void loadPlugin(
            Vamp::HostExt::PluginLoader::PluginKey key,
            float inputSampleRate,
            int adapterFlags = 0);

    const std::string& getIdentifier() const {
        DEBUG_ASSERT(m_plugin);
        return m_identifier;
    }

    const std::string& getName() const {
        DEBUG_ASSERT(m_plugin);
        return m_name;
    }

    const Vamp::Plugin::OutputList& getOutputDescriptors() const {
        DEBUG_ASSERT(m_plugin);
        return m_outputDescriptors;
    }

    size_t getPreferredBlockSize() const {
        DEBUG_ASSERT(m_plugin);
        return m_preferredBlockSize;
    }

    size_t getPreferredStepSize() const {
        DEBUG_ASSERT(m_plugin);
        return m_preferredStepSize;
    }

    bool initialise(
            size_t inputChannels,
            size_t stepSize,
            size_t blockSize);

    Vamp::Plugin::FeatureSet process(
            const float* const* inputBuffers,
            Vamp::RealTime timestamp);
    Vamp::Plugin::FeatureSet getRemainingFeatures();

  private:
    // This pointer must never be deleted implicitly to avoid race conditions!
    // It is only needed for utilizing the default move constructor.
    std::unique_ptr<Vamp::Plugin> m_plugin;

    std::string m_identifier;
    std::string m_name;
    Vamp::Plugin::OutputList m_outputDescriptors;
    size_t m_preferredBlockSize;
    size_t m_preferredStepSize;
};

} // namespace mixxx
