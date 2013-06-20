/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LADSPALIBRARY_H
#define LADSPALIBRARY_H



#include <QtCore>
#include <ladspa.h>

#include "ladspaplugin.h"

class LADSPALibrary
{
public:
    LADSPALibrary(QString file);
    ~LADSPALibrary();
    
    const LADSPAPluginList * pluginList();
    int pluginCount();

private:
    QLibrary * m_pLibrary;
    LADSPAPluginList m_Plugins;
    QString m_qFilePath;

    LADSPA_Descriptor_Function m_descriptorFunction;
};

typedef QList<LADSPALibrary *> LADSPALibraryList;

#endif
