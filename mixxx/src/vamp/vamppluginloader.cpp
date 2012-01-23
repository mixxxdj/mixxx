/*
 * vamppluginloader.cpp
 *
 *  Created on: 23/jan/2012
 *      Author: Tobias Rafreider
 *
 * This is a thread-safe wrapper class around Vamp's
 * PluginLoader class.
 */
#include "vamppluginloader.h"

using Vamp::Plugin;
using Vamp::PluginHostAdapter;

VampPluginLoader* VampPluginLoader::m_instance = 0;
static QMutex sMutex;


VampPluginLoader::VampPluginLoader()
        :m_mutex(QMutex::Recursive){
    m_pVampPluginLoaderDelegee = Vamp::HostExt::PluginLoader::getInstance();
}

VampPluginLoader::~VampPluginLoader() {

}

VampPluginLoader* VampPluginLoader::getInstance(){

    if( !m_instance )
    {
        sMutex.lock();
        if(!m_instance )
            m_instance = new VampPluginLoader();
        sMutex.unlock();
    }
    return m_instance;
}

PluginLoader::PluginKey VampPluginLoader::composePluginKey(std::string libraryName, std::string identifier){
    QMutexLocker lock(&m_mutex);
    PluginLoader::PluginKey key = m_pVampPluginLoaderDelegee->composePluginKey(libraryName, identifier);
    return key;

}

PluginLoader::PluginCategoryHierarchy VampPluginLoader::getPluginCategory(Vamp::HostExt::PluginLoader::PluginKey plugin){
    QMutexLocker lock(&m_mutex);
    return m_pVampPluginLoaderDelegee->getPluginCategory(plugin);
}

PluginLoader::PluginKeyList VampPluginLoader::listPlugins(){
    QMutexLocker lock(&m_mutex);
    return m_pVampPluginLoaderDelegee->listPlugins();
}

Vamp::Plugin* VampPluginLoader::loadPlugin(Vamp::HostExt::PluginLoader::PluginKey key, float inputSampleRate, int adapterFlags){
    QMutexLocker lock(&m_mutex);
    return m_pVampPluginLoaderDelegee->loadPlugin(key, inputSampleRate, adapterFlags);
}




