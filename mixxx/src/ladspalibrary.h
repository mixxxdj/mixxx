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

#include <ladspaplugin.h>

#include <qlibrary.h>
#include <q3ptrlist.h>
#include <qstring.h>

#include <ladspa.h>

class LADSPALibrary {

public:
    LADSPALibrary(QString file);
    ~LADSPALibrary();
    
    const LADSPAPluginList * pluginList();
    int pluginCount();

protected:
    QLibrary * m_pLibrary;
    LADSPAPluginList m_Plugins;
    QString m_qFilePath;

    LADSPA_Descriptor_Function m_descriptorFunction;
};

typedef Q3PtrList<LADSPALibrary> LADSPALibraryList;

#endif
