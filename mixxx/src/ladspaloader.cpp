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

    QString ladspaPath = QString(getenv("LADSPA_PATH"));
    QStringList paths;

    if (!ladspaPath.isEmpty())
    {
        // get the list of directories containing LADSPA plugins
#ifdef __WIN32__
        paths = ladspaPath.split(';');
#else
        paths = ladspaPath.split(':');
#endif
    }
    else
    {
        // add default path if LADSPA_PATH is not set
#ifdef __LINUX__
        paths.push_back ("/usr/lib/ladspa/");
#elif __MACX__
        paths.push_back ("/Library/Audio/Plug-ins/LADSPA");
#elif __WIN32__
        // not tested yet but should work:
        QString programFiles = QString(getenv("ProgramFiles"));
        paths.push_back (programFiles+"\\LADSPA Plugins");
        paths.push_back (programFiles+"\\Audacity\\Plug-Ins");
#endif
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
