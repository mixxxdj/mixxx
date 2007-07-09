/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <ladspaplugin.h>

LADSPAPlugin::LADSPAPlugin(const LADSPA_Descriptor * descriptor)
{
    m_pDescriptor = descriptor;
}

LADSPAPlugin::~LADSPAPlugin()
{
    // TODO
}

LADSPAInstance * LADSPAPlugin::instantiate()
{
    LADSPAInstance * instance = new LADSPAInstance(m_pDescriptor);
    return instance;
}
