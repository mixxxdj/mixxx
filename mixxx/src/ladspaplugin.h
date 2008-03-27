/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LADSPAPLUGIN_H
#define LADSPAPLUGIN_H

#include <QtCore>
#include <q3ptrlist.h>
#include <q3ptrvector.h>

#include <ladspa.h>

#include "ladspainstance.h"

class LADSPAPlugin
{
public:
    LADSPAPlugin(const LADSPA_Descriptor * descriptor);
    ~LADSPAPlugin();

    LADSPAInstance * instantiate();
    const char * getLabel();

private:
    const LADSPA_Descriptor * m_pDescriptor;
};

typedef Q3PtrList<LADSPAPlugin> LADSPAPluginList;
typedef Q3PtrVector<LADSPAPlugin> LADSPAPluginVector;

#endif
