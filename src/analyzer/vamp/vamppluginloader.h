#ifndef MIXXX_VAMPPLUGINLOADER_H
#define MIXXX_VAMPPLUGINLOADER_H

#include <vamp-hostsdk/vamp-hostsdk.h>


namespace mixxx {

class VampPluginLoader final {
  public:
    VampPluginLoader();

    Vamp::HostExt::PluginLoader::PluginKeyList listPlugins();
    Vamp::Plugin *loadPlugin(Vamp::HostExt::PluginLoader::PluginKey,
                             float inputSampleRate, int adapterFlags = 0);
    Vamp::HostExt::PluginLoader::PluginKey composePluginKey(std::string libraryName,
                                             std::string identifier);
    Vamp::HostExt::PluginLoader::PluginCategoryHierarchy getPluginCategory(
        Vamp::HostExt::PluginLoader::PluginKey plugin);
};

} // namespace mixxx


#endif // MIXXX_VAMPPLUGINLOADER_H
