/*
 * vamppluginloader.h
 *
 *  Created on: 23/jan/2012
 *      Author: Tobias Rafreider
 *
 * This is a thread-safe wrapper class around Vamp's
 * PluginLoader class.
 */

#ifndef ANALYZER_VAMP_VAMPPLUGINLOADER_H
#define ANALYZER_VAMP_VAMPPLUGINLOADER_H

#include <QMutex>
#include <vamp-hostsdk/vamp-hostsdk.h>
#include <string>

#include "util/class.h"

using Vamp::Plugin;
using Vamp::PluginHostAdapter;
using Vamp::HostExt::PluginLoader;
using Vamp::HostExt::PluginWrapper;
using Vamp::HostExt::PluginInputDomainAdapter;

class VampPluginLoader {
  private:
    VampPluginLoader();
    virtual ~VampPluginLoader();

  public:
    static VampPluginLoader* getInstance();
    PluginLoader::PluginKeyList listPlugins();
    Vamp::Plugin *loadPlugin(Vamp::HostExt::PluginLoader::PluginKey,
                             float inputSampleRate, int adapterFlags = 0);
    PluginLoader::PluginKey composePluginKey(std::string libraryName,
                                             std::string identifier);
    PluginLoader::PluginCategoryHierarchy getPluginCategory(
        Vamp::HostExt::PluginLoader::PluginKey plugin);

  private:
    static VampPluginLoader* s_instance;
    static QMutex s_mutex;
    Vamp::HostExt::PluginLoader* m_pVampPluginLoader;
    DISALLOW_COPY_AND_ASSIGN(VampPluginLoader);
};

#endif // ANALYZER_VAMP_VAMPPLUGINLOADER_H
