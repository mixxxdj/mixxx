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
#include "engine/engineladspa.h"
#include "ladspaloader.h"

#include <QtCore>
#include <QtXml>

LADSPAPreset::LADSPAPreset()
{
}

LADSPAPreset::LADSPAPreset(QDomElement element, LADSPALoader * loader)
{
    m_qName = element.tagName();

    QDomNodeList pluginNodeList = element.elementsByTagName(QString("Plugin"));
    m_Plugins.resize(pluginNodeList.count());
    int j = 0;
    for (int i = 0; i < pluginNodeList.count(); i++)
    {
        QDomElement pluginElement = pluginNodeList.item(i).toElement();
        if (pluginElement.parentNode() != element)
        {
            continue;
        }
        LADSPAPlugin * plugin = loader->getByLabel(pluginElement.text());
	if (plugin == NULL)
	{
	    m_bValid = false;
	    qDebug() << "LADSPA: Plugin " << pluginElement.text() << " not found (required by preset " << m_qName << ")";
	    return; // ?
	}
        m_Plugins[j] = plugin;
	j++;
    }
    m_Plugins.resize(j);

    QDomNodeList knobNodeList = element.elementsByTagName(QString("Knob"));
    m_Knobs.resize(knobNodeList.count());
    for (int i = 0; i < knobNodeList.count(); i++)
    {
        QDomElement knobElement = knobNodeList.item(i).toElement();
        LADSPAPresetKnob * knob = new LADSPAPresetKnob(knobElement);
        m_Knobs[i] = knob;
    }

    m_bValid = true;
}

LADSPAPreset::~LADSPAPreset()
{
}

LADSPAPresetInstance * LADSPAPreset::instantiate(int slot)
{
    LADSPAPresetInstance * instance = new LADSPAPresetInstance(m_Plugins.count(), m_Knobs.count(), slot);

    EngineLADSPA * engine = EngineLADSPA::getEngine();

    for (int i = 0; i < m_Plugins.count(); i++)
    {
        instance->addPlugin(i, m_Plugins[i], engine);
    }

    for (int i = 0; i < m_Knobs.count(); i++)
    {
        instance->addControl(i, m_Knobs[i], engine);
    }

    return instance;
}

QString LADSPAPreset::getName()
{
    return m_qName;
}

bool LADSPAPreset::isValid()
{
    return m_bValid;
}
