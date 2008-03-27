/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "ladspapreset.h"
#include "ladspapresetinstance.h"
#include "engineladspa.h"
#include "ladspaloader.h"

#include <QtCore>
#include <QtXml>
#include <QDomElement>
#include <QDomNodeList>

LADSPAPreset::LADSPAPreset()
{
}

LADSPAPreset::LADSPAPreset(QDomElement element, LADSPALoader * loader)
{
    m_qName = element.tagName();

    QDomNodeList pluginNodeList = element.elementsByTagName(QString("Plugin"));
    m_Plugins.resize(pluginNodeList.count());
    for (int i = 0; i < pluginNodeList.count(); i++)
    {
        QDomElement pluginElement = pluginNodeList.item(i).toElement();
        if (pluginElement.parentNode() != element)
        {
            continue;
        }
        LADSPAPlugin * plugin = loader->getByLabel(pluginElement.text().toAscii().constData());
        m_Plugins.insert(i, plugin);
    }

    QDomNodeList knobNodeList = element.elementsByTagName(QString("Knob"));
    m_Knobs.resize(knobNodeList.count());
    for (int i = 0; i < knobNodeList.count(); i++)
    {
        QDomElement knobElement = knobNodeList.item(i).toElement();
        LADSPAPresetKnob * knob = new LADSPAPresetKnob(knobElement);
        m_Knobs.insert(i, knob);
    }
}

LADSPAPreset::~LADSPAPreset()
{
}

LADSPAPresetInstance * LADSPAPreset::instantiate()
{
    LADSPAPresetInstance * instance = new LADSPAPresetInstance(m_Plugins.count(), m_Knobs.count());

    EngineLADSPA * engine = EngineLADSPA::getEngine();

    for (unsigned int i = 0; i < m_Plugins.count(); i++)
    {
        instance->addPlugin(i, m_Plugins[i], engine);
    }

    for (unsigned int i = 0; i < m_Knobs.count(); i++)
    {
        instance->addControl(i, m_Knobs[i], engine);
    }

    return instance;
}

QString LADSPAPreset::getName()
{
    return m_qName;
}
