/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "ladspapresetknob.h"

#include <QtCore>
#include <QtXml>

LADSPAPresetKnob::LADSPAPresetKnob()
{
}

LADSPAPresetKnob::LADSPAPresetKnob(QDomElement element)
{
    QDomElement labelElement = element.firstChildElement(QString("Label"));
    QDomElement minElement = element.firstChildElement(QString("Min"));
    QDomElement maxElement = element.firstChildElement(QString("Max"));
    QDomElement defaultElement = element.firstChildElement(QString("Default"));
    m_qLabel = labelElement.text();
    m_fMin = minElement.text().toFloat();
    m_fMax = maxElement.text().toFloat();
    m_fDefault = defaultElement.text().toFloat();

    QDomNodeList connectionNodeList = element.elementsByTagName(QString("Connection"));
    m_Connections.resize(connectionNodeList.count());
    for (int i = 0; i < connectionNodeList.count(); i++)
    {
        QDomElement connectionElement = connectionNodeList.item(i).toElement();
        QDomElement pluginElement = connectionElement.firstChildElement(QString("Plugin"));
        QDomElement portElement = connectionElement.firstChildElement(QString("Port"));
        m_Connections[i].plugin = pluginElement.text().toULong();
        m_Connections[i].port = portElement.text().toULong();
    }
}

LADSPAPresetKnob::~LADSPAPresetKnob()
{
}

LADSPAPortConnectionVector * LADSPAPresetKnob::getConnections()
{
    return &m_Connections;
}

QString LADSPAPresetKnob::getLabel()
{
    return m_qLabel;
}

LADSPA_Data LADSPAPresetKnob::getMin()
{
    return m_fMin;
}

LADSPA_Data LADSPAPresetKnob::getMax()
{
    return m_fMax;
}

LADSPA_Data LADSPAPresetKnob::getDefault()
{
    return m_fDefault;
}
