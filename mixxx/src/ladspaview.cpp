/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "ladspaview.h"

#include <QtCore>
#include <QtGui>

#include "ladspapresetmanager.h"
#include "ladspapresetinstance.h"
#include "ladspapresetslot.h"
#include "wknob.h"
#include "wskincolor.h"
#include "wpixmapstore.h"

LADSPAView::LADSPAView(QWidget * parent) : QWidget(parent, "LADSPA")
{
    QDomDocument skin("LADSPASkin");
    QFile file(WWidget::getPath("ladspa_skin.xml"));
    if (!file.open(IO_ReadOnly))
    {
        qDebug() << "Could not open skin definition file: " << file.fileName();
    }
    if (!skin.setContent(&file))
    {
        qDebug() << "Error parsing skin definition file: " << file.fileName();
    }
    file.close();
    QDomElement docElement = skin.documentElement();

    QDomElement bgElement = docElement.firstChildElement("Background");
    QString filename = bgElement.firstChildElement("Path").text();
    QPixmap *background = WPixmapStore::getPixmapNoCache(WWidget::getPath(filename));
    QLabel *bg = new QLabel(this);

    bg->move(0, 0);
    bg->setPixmap(*background);
    bg->lower();
    this->setFixedSize(background->width(), background->height());
    parent->setMinimumSize(background->width(), background->height());
    
    QDomElement bgColorNode = docElement.firstChildElement("BgColor");
    QDomElement fgColorNode = docElement.firstChildElement("FgColor");

    QPalette palette;
    QColor c(0,0,0);
    c.setNamedColor(bgColorNode.text());
    palette.setBrush(QPalette::Window, WSkinColor::getCorrectColor(c));
    QColor c2(255,255,255);
    c2.setNamedColor(fgColorNode.text());
    palette.setBrush(foregroundRole(), WSkinColor::getCorrectColor(c2));
    setBackgroundRole(QPalette::Window);
    setPalette(palette);
    setAutoFillBackground(true);

    palette.setColor(QPalette::Base, WSkinColor::getCorrectColor(c));
    palette.setColor(QPalette::Text, WSkinColor::getCorrectColor(c2));

    m_pPresetList = new QListWidget(this);
    m_pPresetList->setDragEnabled(true);

    QDomElement presetListElement = docElement.firstChildElement("PresetList");

    QDomElement posElement = presetListElement.firstChildElement("Pos");
    QString pos = posElement.text();
    int x = pos.left(pos.indexOf(",")).toInt();
    int y = pos.mid(pos.indexOf(",") + 1).toInt();
    if (x < 0)
    {
        x = this->width() + x;
    }
    if (y < 0)
    {
	y = this->height() + y;
    }
    m_pPresetList->move(x, y);

    QDomElement sizeElement = presetListElement.firstChildElement("Size");
    QString size = sizeElement.text();
    int width = size.left(size.indexOf(",")).toInt();
    int height = size.mid(size.indexOf(",") + 1).toInt();
    if (width <= 0)
    {
	width = this->width() + width;
    }
    if (height <= 0)
    {
	height = this->height() + height;
    }    
    m_pPresetList->resize(width, height);

    m_pPresetManager = new LADSPAPresetManager();

    for (unsigned int i = 0; i < m_pPresetManager->getPresetCount(); i++)
    {
        m_pPresetList->addItem(m_pPresetManager->getPreset(i)->getName());
    }

    m_pSlotTable = new QWidget(this);

    QDomElement slotTableElement = docElement.firstChildElement("SlotTable");

    posElement = slotTableElement.firstChildElement("Pos");
    pos = posElement.text();
    x = pos.left(pos.indexOf(",")).toInt();
    y = pos.mid(pos.indexOf(",") + 1).toInt();
    if (x < 0)
    {
        x = this->width() + x;
    }
    if (y < 0)
    {
	y = this->height() + y;
    }
    m_pSlotTable->move(x, y);

    sizeElement = slotTableElement.firstChildElement("Size");
    size = sizeElement.text();
    width = size.left(size.indexOf(",")).toInt();
    height = size.mid(size.indexOf(",") + 1).toInt();
    if (width <= 0)
    {
	width = this->width() + width;
    }
    if (height <= 0)
    {
	height = this->height() + height;
    }    
    m_pSlotTable->resize(width, height);

    m_pSlotTable->show();

    QDomElement slotsElement = slotTableElement.firstChildElement("Slots");
    int numberOfSlots = slotsElement.text().toInt();

    QDomElement slotElement = slotTableElement.firstChildElement("Slot");
    for (int i = 0; i < numberOfSlots; i++)
    {
	LADSPAPresetSlot *p = new LADSPAPresetSlot(m_pSlotTable, slotElement, i, m_pPresetManager, palette);
	p->show();
    }

    m_pPresetList->setBackgroundRole(QPalette::Window);
    m_pPresetList->setPalette(palette);
    m_pPresetList->setAutoFillBackground(true);
    parent->setPalette(palette);
}

LADSPAView::~LADSPAView()
{
}
