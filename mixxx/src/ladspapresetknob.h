/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LADSPAPRESETKNOB_H
#define LADSPAPRESETKNOB_H

#include <QtCore>
#include <QtXml>

#include <ladspa.h>

class QDomElement;

struct LADSPAPortConnection
{
    unsigned long plugin, port;
};

typedef QVector<LADSPAPortConnection> LADSPAPortConnectionVector;

class LADSPAPresetKnob
{
public:
    LADSPAPresetKnob();
    LADSPAPresetKnob(QDomElement element);
    ~LADSPAPresetKnob();

    LADSPAPortConnectionVector * getConnections();
    QString getLabel();
    LADSPA_Data getMin();
    LADSPA_Data getMax();
    LADSPA_Data getDefault();

private:
    QString m_qLabel;
    LADSPAPortConnectionVector m_Connections;
    LADSPA_Data m_fMin, m_fMax, m_fDefault;
};

typedef QVector<LADSPAPresetKnob *> LADSPAPresetKnobVector;

#endif
