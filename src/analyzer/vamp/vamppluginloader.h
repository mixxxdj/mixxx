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

    Vamp::HostExt::PluginLoader::PluginKeyList listPlugins();
    Vamp::Plugin *loadPlugin(Vamp::HostExt::PluginLoader::PluginKey,
                             float inputSampleRate, int adapterFlags = 0);
    void unloadPlugin(Vamp::Plugin** ppPlugin);

    bool loadAnalyzerPlugin(
            VampAnalyzer* pAnalyzer,
            const QString& pluginLib,
            const QString& pluginId,
            SINT inputChannels,
            SINT inputSampleRate);

    void unloadAnalyzerPlugin(
            VampAnalyzer* pAnalyzer);
};

} // namespace mixxx


#endif // MIXXX_VAMPPLUGINLOADER_H
