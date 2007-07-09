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
    fprintf(stderr, "Loading LADSPA Library \e[91m%s\e[0m:\e[92m", file.ascii()); // DEBUG
    m_pLibrary = new QLibrary(file);
    m_qFilePath = file;

    if (m_pLibrary->load() == TRUE)
    {
        m_descriptorFunction = (LADSPA_Descriptor_Function) m_pLibrary->resolve("ladspa_descriptor");

        const LADSPA_Descriptor * descriptor;

        for (unsigned long index = 0; (descriptor = m_descriptorFunction(index)) != NULL; index++)
        {
            fprintf (stderr, " %lu", index); // DEBUG
            LADSPAPlugin * plugin = new LADSPAPlugin(descriptor);
            m_Plugins.append(plugin);
        }
    }
    fprintf (stderr, "\e[0m... loaded.\n"); // DEBUG
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
