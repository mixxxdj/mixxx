/*
 * vamppluginloader.cpp
 *
 *  Created on: 23/jan/2012
 *      Author: Tobias Rafreider
 *
 * This is a thread-safe wrapper class around Vamp's
 * PluginLoader class.
 */

#include "vamp/vamppluginloader.h"

using Vamp::Plugin;
using Vamp::PluginHostAdapter;

VampPluginLoader* VampPluginLoader::s_instance = NULL;
QMutex VampPluginLoader::s_mutex;

VampPluginLoader::VampPluginLoader() {
    m_pVampPluginLoader = Vamp::HostExt::PluginLoader::getInstance();
}

VampPluginLoader::~VampPluginLoader() {
}

VampPluginLoader* VampPluginLoader::getInstance() {
    QMutexLocker lock(&s_mutex);
    if (!s_instance) {
        s_instance = new VampPluginLoader();
    }
    return s_instance;
}

PluginLoader::PluginKey VampPluginLoader::composePluginKey(
    std::string libraryName, std::string identifier) {
    QMutexLocker lock(&s_mutex);
    PluginLoader::PluginKey key = m_pVampPluginLoader->composePluginKey(
        libraryName, identifier);
    return key;
}

PluginLoader::PluginCategoryHierarchy VampPluginLoader::getPluginCategory(
    Vamp::HostExt::PluginLoader::PluginKey plugin) {
    QMutexLocker lock(&s_mutex);
    return m_pVampPluginLoader->getPluginCategory(plugin);
}

PluginLoader::PluginKeyList VampPluginLoader::listPlugins() {
    QMutexLocker lock(&s_mutex);
    return m_pVampPluginLoader->listPlugins();
}

Vamp::Plugin* VampPluginLoader::loadPlugin(
    Vamp::HostExt::PluginLoader::PluginKey key,
    float inputSampleRate, int adapterFlags) {
    QMutexLocker lock(&s_mutex);
    return m_pVampPluginLoader->loadPlugin(
        key, inputSampleRate, adapterFlags);
}




