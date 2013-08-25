/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtCore>
#include <qapplication.h>
#include "ladspaloader.h"

LADSPALoader::LADSPALoader()
{
    m_PluginCount = 0;
    
    QStringList plugin_paths;

    // set up the plugin paths...

    qDebug() << "Setting up plugin paths...";

    QString ladspaPath = QString(getenv("LADSPA_PATH"));

    // Is this really necessary? Can't we add all the paths?
    if (!ladspaPath.isEmpty())
    {
            // get the list of directories containing LADSPA plugins
    #ifdef __WINDOWS__
            plugin_paths.ladspaPath.split(';');
    #else  //this doesn't work, I think we need to iterate over the splitting to do it properly
            plugin_paths = ladspaPath.split(':');
    #endif
    }
    else
    {
            // add default path if LADSPA_PATH is not set
    #ifdef __LINUX__
            plugin_paths.push_back ("/usr/lib/ladspa/");
            plugin_paths.push_back ("/usr/lib64/ladspa/");
    #elif __APPLE__
            QDir dir(QCoreApplication::applicationDirPath());
            dir.cdUp();
            dir.cd("PlugIns");
             plugin_paths.push_back ("/Library/Audio/Plug-ins/LADSPA");
             plugin_paths.push_back (dir.absolutePath()); //ladspa_plugins directory in Mixxx.app bundle //XXX work in QApplication::appdir()
    #elif __WINDOWS__
            // not tested yet but should work:
            QString programFiles = QString(getenv("ProgramFiles"));
             plugin_paths.push_back (programFiles+"\\LADSPA Plugins");
             plugin_paths.push_back (programFiles+"\\Audacity\\Plug-Ins");
    #endif
    }
    qDebug() << "...done.";

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

LADSPAPlugin * LADSPALoader::getByIndex(uint index)
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
