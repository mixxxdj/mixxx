/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LADSPAPRESET_H
#define LADSPAPRESET_H

#include <QtCore>
#include <QtXml>

#include "ladspaplugin.h"
#include "ladspapresetknob.h"

class QDomElement;

class LADSPAPresetInstance;
class LADSPALoader;

class LADSPAPreset
{
public:
    LADSPAPreset();
    LADSPAPreset(QDomElement element, LADSPALoader * loader);
    ~LADSPAPreset();

    LADSPAPresetInstance * instantiate(int slot);
    QString getName();

    bool isValid();

private:
    QString m_qName;
    LADSPAPluginVector m_Plugins;
    LADSPAPresetKnobVector m_Knobs;
    bool m_bValid;
};

typedef QVector<LADSPAPreset *> LADSPAPresetVector;

#endif
