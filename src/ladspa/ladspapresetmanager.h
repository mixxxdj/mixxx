/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LADSPAPRESETMANAGER_H
#define LADSPAPRESETMANAGER_H

#include "ladspapreset.h"

#define LADSPA_PRESET_PATH "ladspa_presets"

class LADSPAPresetManager
{
public:
    LADSPAPresetManager();
    ~LADSPAPresetManager();
        
    unsigned int getPresetCount();
    LADSPAPreset * getPreset(unsigned int i);

private:
    LADSPAPresetVector m_Presets;
    unsigned int m_iPresetCount;
};

#endif
