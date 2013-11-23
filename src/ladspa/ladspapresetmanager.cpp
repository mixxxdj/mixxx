/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtCore>
#include <QtXml>

#include "ladspapresetmanager.h"
#include "ladspaloader.h"
#include "widget/wwidget.h"


LADSPAPresetManager::LADSPAPresetManager()
{
    m_iPresetCount = 0;

    LADSPALoader * loader = new LADSPALoader();

    QDir dir(WWidget::getPath(QString("../../ladspa_presets"))); // TODO
    QFileInfoList files = dir.entryInfoList();
    m_Presets.resize(files.count());
    for (QFileInfoList::iterator fileIter = files.begin(); fileIter != files.end(); fileIter++)
    {
        qDebug() << "LADSPA: file " << (*fileIter).filePath();
        if ((*fileIter).isDir())
        {
            continue;
        }

        QFile file((* fileIter).filePath());

        QDomDocument document("Preset");
        file.open(QIODevice::ReadOnly);
        document.setContent(&file);
        file.close();

        LADSPAPreset * preset = new LADSPAPreset(document.documentElement(), loader);
        m_Presets [m_iPresetCount] = preset;
        m_iPresetCount++;
    }
}

LADSPAPresetManager::~LADSPAPresetManager()
{
}

unsigned int LADSPAPresetManager::getPresetCount()
{
    return m_iPresetCount;
}

LADSPAPreset * LADSPAPresetManager::getPreset(unsigned int i)
{
    if (i >= m_iPresetCount)
    {
        return NULL;
    }
    return m_Presets[i];
}
