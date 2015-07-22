/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QApplication>
#include "ladspaloader.h"

LADSPALoader::LADSPALoader()
{
    m_PluginCount = 0;

    QStringList plugin_paths;


    // load each directory
    for (QStringList::iterator path = plugin_paths.begin(); path != plugin_paths.end(); path++)
    {
        QDir dir(* path);

        qDebug() << "Looking for plugins in directory:" << dir.absolutePath();

        // get the list of files in the directory
        QFileInfoList files = dir.entryInfoList();

        // load each file in the directory
        for (QFileInfoList::iterator file = files.begin(); file != files.end(); file++)
        {
            qDebug() << "Looking at file:" << (*file).absoluteFilePath();

            if ((*file).isDir())
            {
                continue;
            }

            try {
                LADSPALibrary * library = new LADSPALibrary ((*file).absoluteFilePath());

                // add the library to the list of all libraries
                m_Libraries.append (library);

                const LADSPAPluginList * plugins = library->pluginList();

                //m_Plugins.resize(m_PluginCount + library->pluginCount());

                // add each plugin in the library to the vector of all plugins
                for (LADSPAPluginList::const_iterator plugin = plugins->begin(); plugin != plugins->end(); plugin++)
                {
                    m_PluginCount++;
                    m_Plugins.push_back(*plugin);
                }
            } catch (QString& s) {
                qDebug() << s;
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

LADSPAPlugin * LADSPALoader::getByLabel(QString label)
{
    // TODO: trie or hash map
    for (unsigned int i = 0; i < m_PluginCount; i++)
    {
      if (QString::compare(m_Plugins[i]->getLabel(), label) == 0)
        {
            return m_Plugins[i];
        }
    }

    return NULL;
}
