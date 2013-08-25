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

#include <ladspa.h>

#include "ladspainstance.h"

class LADSPAPlugin
{
public:
    LADSPAPlugin(const LADSPA_Descriptor * descriptor);
    ~LADSPAPlugin();

    LADSPAInstance * instantiate(int slot);
    const QString getLabel();
    const LADSPA_Descriptor * getDescriptor();
    bool isSupported();

private:
    const LADSPA_Descriptor * m_pDescriptor;
};

typedef QList<LADSPAPlugin *> LADSPAPluginList;
typedef QVector<LADSPAPlugin *> LADSPAPluginVector;

#endif
