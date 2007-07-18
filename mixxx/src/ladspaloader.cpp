/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <ladspaloader.h>

#include <qstringlist.h>
#include <qdir.h>
#include <qfileinfo.h>

LADSPALoader::LADSPALoader()
{
    m_PluginCount = 0;

    // get the list of directories containing LADSPA plugins
    QStringList paths = QString(getenv("LADSPA_PATH")).split(':');

    // add default path if LADSPA_PATH is not set
    if (paths.isEmpty())
    {
        paths.push_back ("/usr/lib/ladspa/"); // TODO: portability
    }

    // load each directory
    for (QStringList::iterator path = paths.begin(); path != paths.end(); path++)
    {
        QDir dir(*path);

        // get the list of files in the directory
        QFileInfoList files = dir.entryInfoList();

        // load each file in the directory
        for (QFileInfoList::iterator file = files.begin(); file != files.end(); file++)
        {
            if ((*file).isDir())
            {
                continue;
            }
            LADSPALibrary *library = new LADSPALibrary ((*file).absoluteFilePath());

            // add the library to the list of all libraries
            m_Libraries.append (library);

            const LADSPAPluginList * plugins = library->pluginList();

            m_Plugins.resize(m_PluginCount + library->pluginCount());

            // add each plugin in the library to the vector of all plugins
            for (LADSPAPluginList::iterator plugin = plugins->begin(); plugin != plugins->end(); plugin++)
            {
                m_PluginCount++;
                m_Plugins.insert(m_PluginCount - 1, *plugin);
            }
        }
    }
}

LADSPALoader::~LADSPALoader()
{
    // TODO: unload & free everything
}

const LADSPAPlugin * LADSPALoader::getByIndex(uint index)
{
    if (index < m_PluginCount)
    {
        return m_Plugins[index];
    }

    return NULL;
}

LADSPAPlugin * LADSPALoader::getByLabel(const char * label)
{
    // TODO: trie or hash map
    for (unsigned int i = 0; i < m_PluginCount; i++)
    {
        if (strcmp (m_Plugins[i]->getLabel(), label) == 0)
        {
            return m_Plugins[i];
        }
    }

    return NULL;
}
