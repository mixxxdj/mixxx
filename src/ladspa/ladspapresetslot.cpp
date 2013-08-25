/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "ladspapresetslot.h"

#include <QtCore>
#include <QtGui>
#include <QtXml>

#include "ladspapresetmanager.h"
#include "ladspapreset.h"
#include "ladspapresetinstance.h"
#include "widget/wknob.h"
#include "widget/wpushbutton.h"
#include "widget/wlabel.h"
#include "configobject.h"
#include "controlpushbutton.h"
#include "controlpotmeter.h"

LADSPAPresetSlot::LADSPAPresetSlot(QWidget *parent, QDomElement element, int slot, LADSPAPresetManager *presetManager, QPalette palette) : QWidget(parent)
{
    m_pPresetManager = presetManager;
    m_qPalette = palette;
    m_iSlotNumber = slot;

    setAcceptDrops(true);

    m_pScrollArea = new QScrollArea(this);
    m_pScrollWidget = new QWidget(m_pScrollArea);
    m_pScrollArea->setWidget(m_pScrollWidget);
    //m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QDomElement posElement = element.firstChildElement("Pos");
    QString pos = posElement.text();
    int x = pos.left(pos.indexOf(",")).toInt();
    int y = pos.mid(pos.indexOf(",") + 1).toInt();
    if (x < 0)
    {
        x = parent->width() + x;
    }
    if (y < 0)
    {
	    y = parent->height() + y;
    }
    QDomElement spacingElement = element.firstChildElement("Spacing");
    QString spacing = spacingElement.text();
    int spacingWidth = spacing.left(spacing.indexOf(",")).toInt();
    int spacingHeight = spacing.mid(spacing.indexOf(",") + 1).toInt();

    QDomElement sizeElement = element.firstChildElement("Size");
    QString size = sizeElement.text();
    int width = size.left(size.indexOf(",")).toInt();
    int height = size.mid(size.indexOf(",") + 1).toInt();
    if (width <= 0)
    {
	width = parent->width() + width;
    }
    if (height <= 0)
    {
	height = parent->height() + height;
    }
    move(x + slot * spacingWidth, y + slot * (height + spacingHeight));
    resize(width, height);
    m_pScrollArea->resize(width, height);
    m_pScrollWidget->resize(width - 5, height - 5);
    m_iBaseWidth = width;

    QString slotString;
    slotString.setNum(slot);

    QDomNodeList buttonNodeList = element.elementsByTagName("PushButton");
    for (int i = 0; i < buttonNodeList.count(); i++)
    {
	QDomElement buttonElement = buttonNodeList.item(i).toElement();
	QString configKey = buttonElement.firstChildElement("Connection").firstChildElement("ConfigKey").text();
	if (configKey.startsWith("[LADSPA],RemoveEffect"))
	{
	    QString keyString = QString("RemoveEffect") + slotString;
	    ConfigKey *key = new ConfigKey("[LADSPA]", keyString);
	    ControlPushButton *control = new ControlPushButton(*key);
	    m_pRemoveButton = new WPushButton(m_pScrollWidget);
	    buttonElement.firstChildElement("Connection").firstChildElement("ConfigKey").firstChild().setNodeValue("[LADSPA]," + keyString);
	    m_pRemoveButton->setup(buttonElement);
	    m_pRemoveButton->setVisible(false);
	}
	else if (configKey.startsWith("[LADSPA],EnableEffect"))
	{
        qDebug() << "Setting up LADSPA EnableEffect" << slotString;
	    QString keyString = QString("EnableEffect") + slotString;
	    ConfigKey *key = new ConfigKey("[LADSPA]", keyString);
        qDebug() << "Key string:" << keyString;
	    ControlObject *control = new ControlPushButton(*key);
        control->set(1.0f);
	    m_pEnableButton = new WPushButton(m_pScrollWidget);
	    buttonElement.firstChildElement("Connection").firstChildElement("ConfigKey").firstChild().setNodeValue("[LADSPA]," + keyString);
	    m_pEnableButton->setup(buttonElement);
	}
	else
	{
	    qDebug() << "LADSPA: Invalid skin (unknown button: " << configKey << ")";
	}
    }

    QDomElement labelElement = element.firstChildElement("Label");
    m_pLabel = new QLabel(m_pScrollWidget);
    m_pLabel->setText(tr("Drag a preset from the list & drop it here"));

    posElement = labelElement.firstChildElement("Pos");
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
    m_pLabel->move(x, y);

    QDomElement widthElement = labelElement.firstChildElement("Width");
    if (!(widthElement.isNull()))
    {
	    width = widthElement.text().toInt();
	    m_pLabel->setMaximumWidth(width);
    }

    m_pLabel->setPalette(palette);

    m_qKnobElement = element.firstChildElement("Knob");
    m_pPresetInstance = NULL;


    ConfigKey *key = new ConfigKey("[LADSPA]", "DryWet" + slotString);
    ControlPotmeter *control = new ControlPotmeter(*key, 0.0, 1.0);
    m_pDryWetKnob = new WKnob(m_pScrollWidget);
    QString keyString = QString("[LADSPA],DryWet") + slotString;
    m_qKnobElement.firstChildElement(QString("Connection")).firstChildElement(QString("ConfigKey")).firstChild().setNodeValue(keyString);
    m_qKnobElement.firstChildElement(QString("Tooltip")).firstChild().setNodeValue("Dry/wet");
    m_pDryWetKnob->setup(m_qKnobElement);
    m_pDryWetKnob->show();

    posElement = m_qKnobElement.firstChildElement("Pos");
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
    m_pDryWetKnob->move(x, y);

    spacingElement = m_qKnobElement.firstChildElement("Spacing");
    spacing = spacingElement.text();
    spacingWidth = spacing.left(spacing.indexOf(",")).toInt();

    m_pDryWetLabel = new QLabel("Dry/wet", m_pScrollWidget);
    m_pDryWetLabel->setMaximumWidth(spacingWidth + m_pDryWetKnob->width() - 4);
    m_pDryWetLabel->show();
    x += m_pDryWetKnob->width() / 2 - m_pDryWetLabel->width() / 2;
    m_pDryWetLabel->move(x, y + m_pDryWetKnob->height() + 1);
    m_pDryWetLabel->setPalette(m_qPalette);

    connect(m_pRemoveButton, SIGNAL(valueChangedLeftUp(double)), this, SLOT(slotRemove()));
}

LADSPAPresetSlot::~LADSPAPresetSlot()
{
    if (m_pPresetInstance != NULL)
	unsetPreset();
}

void LADSPAPresetSlot::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
    {
	event->setDropAction(Qt::CopyAction);
	event->accept();
    }
}

void LADSPAPresetSlot::dropEvent(QDropEvent * event)
{
    if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
    {
	int row, column;
	QByteArray encoded = event->mimeData()->data("application/x-qabstractitemmodeldatalist");
	QDataStream stream(&encoded, QIODevice::ReadOnly);
	stream >> row >> column;
	setPreset(m_pPresetManager->getPreset(row));
    }

    event->setDropAction(Qt::CopyAction);
    event->accept();
}

void LADSPAPresetSlot::setPreset(LADSPAPreset *preset)
{
    if (m_pPresetInstance != NULL)
	unsetPreset();

    if (!preset->isValid())
    {
	QMessageBox::warning(0, "LADSPA error", "One or more plugins required by preset '" + preset->getName() + "' are not installed.");
	return;
    }

    m_pLabel->setText(preset->getName());
    m_pRemoveButton->show();

    m_pPresetInstance = preset->instantiate(m_iSlotNumber);

    m_iKnobCount = m_pPresetInstance->getKnobCount();
    m_Knobs.resize(m_iKnobCount);
    m_Labels.resize(m_iKnobCount);
    for (int i = 0; i < m_iKnobCount; i++)
    {
        addKnob(i);
    }
}

void LADSPAPresetSlot::unsetPreset()
{
    m_pLabel->setText("Drag a preset from the list & drop it here");
    m_pRemoveButton->hide();
    m_pScrollWidget->resize(m_iBaseWidth - 5, m_pScrollWidget->height());

    delete m_pPresetInstance;
    m_pPresetInstance = NULL;
    for (int i = 0; i < m_iKnobCount; i++)
    {
        delete m_Knobs[i];
	delete m_Labels[i];
    }
}

void LADSPAPresetSlot::addKnob(int i)
{
    ConfigKey key = m_pPresetInstance->getKey(i);
    WKnob * knob = new WKnob(m_pScrollWidget);
    m_Knobs.insert(i, knob);
    QString keyString = key.group;
    keyString.append(",");
    keyString.append(key.item);
    m_qKnobElement.firstChildElement(QString("Connection")).firstChildElement(QString("ConfigKey")).firstChild().setNodeValue(keyString);
    m_qKnobElement.firstChildElement(QString("Tooltip")).firstChild().setNodeValue(key.item);
    knob->setup(m_qKnobElement);
    knob->show();

    QDomElement posElement = m_qKnobElement.firstChildElement("Pos");
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
    QDomElement spacingElement = m_qKnobElement.firstChildElement("Spacing");
    QString spacing = spacingElement.text();
    int spacingWidth = spacing.left(spacing.indexOf(",")).toInt();
    int spacingHeight = spacing.mid(spacing.indexOf(",") + 1).toInt();
    knob->move(x + (i + 1) * (knob->width() + spacingWidth), y + (i + 1) * spacingHeight);
    if (knob->x() + knob->width() > m_pScrollWidget->width())
    {
	qDebug() << "m_pScrollWidget->resize " << m_pScrollWidget->width() << knob->x() << knob->width() << spacingWidth / 2;
	m_pScrollWidget->resize(knob->x() + knob->width() + spacingWidth, m_pScrollWidget->height());
    }

    int length = key.item.length();
    QLabel * label = new QLabel(key.item, m_pScrollWidget);
    label->setMaximumWidth(spacingWidth + knob->width() - 4);
    label->show();
    /*while (label->width() >= spacingWidth + knob->width() - 5)
    {
	qDebug() << label->width() << ", " << length << ", " << spacingWidth << ", " << knob->width();
	if (length > 10)
	    length = 10;
	length -= 2;
	if (length <= 1)
	    break;
	label->setText(key.item.left(length) + "...");
	label->updateGeometry();
    }*/
    m_Labels.insert(i, label);
    x += knob->width() / 2 - label->width() / 2;
    label->move(x + (i + 1) * (knob->width() + spacingWidth), y + (i + 1) * spacingHeight + knob->height() + 1);
    label->setPalette(m_qPalette);
}

void LADSPAPresetSlot::slotRemove()
{
    unsetPreset();
}
