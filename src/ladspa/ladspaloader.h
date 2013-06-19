/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LADSPALOADER_H
#define LADSPALOADER_H

#include "ladspaplugin.h"
#include "ladspalibrary.h"

class LADSPALoader
{
public:
    LADSPALoader();
    ~LADSPALoader();
        
    LADSPAPlugin * getByIndex(uint index);
    LADSPAPlugin * getByLabel(QString label);

private:
    LADSPAPluginVector m_Plugins;
    LADSPALibraryList m_Libraries;
    unsigned int m_PluginCount;
};

#endif
