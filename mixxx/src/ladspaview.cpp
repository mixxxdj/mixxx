/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <ladspaview.h>

#include <QListWidget>
#include <QScrollArea>
#include <QPushButton>
#include <QColor>
#include <QPalette>
#include <QLabel>

#include <ladspapresetmanager.h>
#include <ladspapresetinstance.h>
#include <wknob.h>
#include <wskincolor.h>

LADSPAView::LADSPAView(QWidget * parent) : QWidget(parent, "LADSPA")
{
    m_iWidth = parent->width();
    m_iListWidth = 150;
    m_iScrollAreaWidth = m_iWidth - m_iListWidth;
    m_iHeight = parent->height();

    resize(m_iWidth, m_iHeight);

    m_pPresetList = new QListWidget(this);
    m_pPresetList->resize(m_iListWidth, m_iHeight);

    m_pScrollArea = new QScrollArea(this);
    m_pScrollArea->resize(m_iScrollAreaWidth, m_iHeight);
    m_pScrollArea->move(m_iListWidth, 0);

    m_pScrollAreaWidget = new QWidget(m_pScrollArea);
    m_pScrollArea->setWidget(m_pScrollAreaWidget);

    m_pScrollAreaWidget->setMinimumSize(m_iScrollAreaWidth - 10, m_iHeight - 10);
    m_pScrollAreaWidget->show();
    m_pScrollArea->show();

    m_pPresetManager = new LADSPAPresetManager();

    m_pWidget = NULL;

    for (unsigned int i = 0; i < m_pPresetManager->getPresetCount(); i++)
    {
        m_pPresetList->addItem(m_pPresetManager->getPreset(i)->getName());
    }

    QDomDocument skin("LADSPASkin");
    QFile file(WWidget::getPath("ladspa_skin.xml"));
    if (!file.open(IO_ReadOnly))
    {
        qDebug("Could not open skin definition file: %s", file.name().latin1());
    }
    if (!skin.setContent(&file))
    {
        qDebug("Error parsing skin definition file: %s", file.name().latin1());
    }
    file.close();
    QDomElement docElem = skin.documentElement();
    m_qKnobSkinNode = docElem.firstChildElement(QString("Knob"));

    if (m_qKnobSkinNode.nodeName()=="Knob")
    {
        qDebug("LADSPA: Skin OK");
    }
    else
    {
        qDebug("LADSPA: Invalid skin (node = %s)", m_qKnobSkinNode.nodeName().latin1());
        return;
    }

    QDomElement bgColorNode = docElem.firstChildElement(QString("BgColor"));
    QDomElement fgColorNode = docElem.firstChildElement(QString("FgColor"));

    QPalette palette;
    QColor c(255,255,255);
    c.setNamedColor(bgColorNode.text());
    palette.setBrush(QPalette::Window, WSkinColor::getCorrectColor(c));
    QColor c2(0,0,0);
    c2.setNamedColor(fgColorNode.text());
    palette.setBrush(foregroundRole(), WSkinColor::getCorrectColor(c2));
    setBackgroundRole(QPalette::Window);
    setPalette(palette);
    setAutoFillBackground(true);

    installEventFilter(this);

    connect(m_pPresetList, SIGNAL(itemSelectionChanged()), this, SLOT(slotSelect()));
}

LADSPAView::~LADSPAView()
{
}

void LADSPAView::slotSelect()
{
    if (m_pWidget != NULL)
    {
        delete m_pWidget;
    }
    m_pWidget = new LADSPAPresetWidget(m_pScrollAreaWidget, m_pPresetManager->getPreset(m_pPresetList->currentRow()), m_qKnobSkinNode);
    m_pWidget->show();
    m_pScrollAreaWidget->show();
    m_pScrollArea->show();
}

LADSPAPresetWidget::LADSPAPresetWidget(QWidget * parent, LADSPAPreset * preset, QDomNode node) : QWidget(parent)
{
    m_pInstance = preset->instantiate();
    m_Knobs.resize(m_pInstance->getKnobCount());
    for (int i = 0; i < m_pInstance->getKnobCount(); i++)
    {
        addKnob(i, m_pInstance->getKey(i), node);
    }
}

LADSPAPresetWidget::~LADSPAPresetWidget()
{
    delete m_pInstance;
    for (int i = 0; i < m_Knobs.count(); i++)
    {
        delete m_Knobs[i];
    }
}

void LADSPAPresetWidget::addKnob(int i, ConfigKey key, QDomNode node)
{
    WKnob * knob = new WKnob(this);
    m_Knobs.insert(i, knob);
    QString keyString = key.group;
    keyString.append(",");
    keyString.append(key.item);
    node.firstChildElement(QString("Connection")).firstChildElement(QString("ConfigKey")).firstChild().setNodeValue(keyString);
    node.firstChildElement(QString("Tooltip")).firstChild().setNodeValue(key.item);
    knob->setup(node);
    knob->show();
    knob->move(i * 50 + 20, 20);
    QString labelString;
    if (key.item.length() > 9)
    {
        labelString = key.item.left(7);
        labelString.append("...");
    }
    else
    {
        labelString = key.item;
    }
    QLabel * label = new QLabel(labelString, this);
    label->show();
    label->move(i * 50 + 20, 25 + knob->height());
}
