/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#include <QtCore>
#include "ladspalibrary.h"

LADSPALibrary::LADSPALibrary(QString file)
{
#ifdef __LINUX__
    //qDebug() << "LADSPA: Loading library \e[91m" << file << "\e[0m:";
#else
    //qDebug() << "LADSPA: Loading library " << file << ":";
#endif
    m_pLibrary = new QLibrary(file);
    m_qFilePath = file;

    if (m_pLibrary->load() == TRUE)
    {
        m_descriptorFunction = (LADSPA_Descriptor_Function) m_pLibrary->resolve("ladspa_descriptor");

		if (m_descriptorFunction == NULL) {
			QString error("The file " + file + " is not a LADSPA plugin");
			throw error;
		}

        const LADSPA_Descriptor * descriptor;

        for (unsigned long index = 0; (descriptor = m_descriptorFunction(index)) != NULL; index++)
        {
            LADSPAPlugin * plugin = new LADSPAPlugin(descriptor);
#ifdef __LINUX__
            //qDebug() << "LADSPA:  " << index << "u - \e[92m" << plugin->getLabel() << "\e[0m";
#else
            //qDebug() << "LADSPA:  " << index << "u - " << plugin->getLabel();
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
