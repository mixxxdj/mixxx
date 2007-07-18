/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <ladspalibrary.h>

LADSPALibrary::LADSPALibrary(QString file)
{
#ifdef __LINUX__
    qDebug("Loading LADSPA Library \e[91m%s\e[0m:", file.toAscii().constData());
#else
    qDebug("Loading LADSPA Library %s:", file.toAscii().constData());
#endif
    m_pLibrary = new QLibrary(file);
    m_qFilePath = file;

    if (m_pLibrary->load() == TRUE)
    {
        m_descriptorFunction = (LADSPA_Descriptor_Function) m_pLibrary->resolve("ladspa_descriptor");

        const LADSPA_Descriptor * descriptor;

        for (unsigned long index = 0; (descriptor = m_descriptorFunction(index)) != NULL; index++)
        {
            LADSPAPlugin * plugin = new LADSPAPlugin(descriptor);
#ifdef __LINUX__
            qDebug(" %lu - \e[92m%s\e[0m", index, plugin->getLabel());
#else
            qDebug(" %lu - %s", index, plugin->getLabel());
#endif
            m_Plugins.append(plugin);
        }
    }
}

LADSPALibrary::~LADSPALibrary()
{
    // TODO
    delete m_pLibrary;
}

const LADSPAPluginList * LADSPALibrary::pluginList()
{
    return &m_Plugins;
}

int LADSPALibrary::pluginCount()
{
    return m_Plugins.count();
}
