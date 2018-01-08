#ifndef MIXXX_VAMPPLUGINLOADER_H
#define MIXXX_VAMPPLUGINLOADER_H

#include <vamp-hostsdk/vamp-hostsdk.h>

#include <QString>

#include "util/types.h"


class VampAnalyzer;

namespace mixxx {

class VampPluginLoader final {
  public:
    VampPluginLoader();

    Vamp::HostExt::PluginLoader::PluginKeyList listPlugins() const;

    Vamp::Plugin *loadPlugin(Vamp::HostExt::PluginLoader::PluginKey,
                             float inputSampleRate, int adapterFlags = 0) const;
    void unloadPlugin(Vamp::Plugin** ppPlugin) const;

    bool loadAnalyzerPlugin(
            VampAnalyzer* pAnalyzer,
            const QString& pluginLib,
            const QString& pluginId,
            SINT inputChannels,
            SINT inputSampleRate) const;

    void unloadAnalyzerPlugin(
            VampAnalyzer* pAnalyzer) const;

    Vamp::Plugin::FeatureSet process(
            Vamp::Plugin* pPlugin,
            const float *const *inputBuffers,
            Vamp::RealTime timestamp) const;
    Vamp::Plugin::FeatureSet getRemainingFeatures(
            Vamp::Plugin* pPlugin) const;
};

} // namespace mixxx


#endif // MIXXX_VAMPPLUGINLOADER_H
