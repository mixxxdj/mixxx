/***************************************************************************
                          wwidget.cpp  -  description
                             -------------------
    begin                : Wed Jun 18 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "wwidget.h"
#include "controlobject.h"

// Static member variable definition
QString WWidget::m_qPath;

WWidget::WWidget(QWidget *parent, const char *name, WFlags flags) : QWidget(parent,name, flags|WStaticContents|WRepaintNoErase|WNoAutoErase)
{
    m_fValue = 0.;
    connect(this, SIGNAL(valueChangedLeftDown(double)), this, SLOT(slotReEmitValueDown(double)));
    connect(this, SIGNAL(valueChangedRightDown(double)), this, SLOT(slotReEmitValueDown(double)));
    connect(this, SIGNAL(valueChangedLeftUp(double)), this, SLOT(slotReEmitValueUp(double)));
    connect(this, SIGNAL(valueChangedRightUp(double)), this, SLOT(slotReEmitValueUp(double)));

    setBackgroundMode(Qt::NoBackground);
}

WWidget::~WWidget()
{
}

void WWidget::setup(QDomNode node)
{
    // Set position
    QString pos = selectNodeQString(node, "Pos");
    int x = pos.left(pos.find(",")).toInt();
    int y = pos.mid(pos.find(",")+1).toInt();
    move(x,y);

    // For each connection
    QDomNode con = selectNode(node, "Connection");
    while (!con.isNull())
    {
        // Get ConfigKey
        ConfigKey configKey;
        QString key = selectNodeQString(con, "ConfigKey");

        configKey.group = key.left(key.find(","));
        configKey.item = key.mid(key.find(",")+1);

        // Get properties from XML, or use defaults
        bool bEmitOnDownPress = true;
        if (selectNodeQString(con, "EmitOnDownPress").contains("false",false))
            bEmitOnDownPress = false;

        Qt::ButtonState state = Qt::NoButton;
        if (!selectNode(con, "ButtonState").isNull())
        {
            if (selectNodeQString(con, "ButtonState").contains("LeftButton"))
                state = Qt::LeftButton;
            else if (selectNodeQString(con, "ButtonState").contains("RightButton"))
                state = Qt::RightButton;
        }

        ControlObject::setWidget(this, configKey, bEmitOnDownPress, state);

        con = con.nextSibling();
    }     
}

void WWidget::setValue(double fValue)
{
    m_fValue = fValue;
    update();
}

void WWidget::slotReEmitValueDown(double fValue)
{
    emit(valueChangedDown(fValue));
}

void WWidget::slotReEmitValueUp(double fValue)
{
    emit(valueChangedUp(fValue));
}

int WWidget::selectNodeInt(const QDomNode &nodeHeader, const QString sNode)
{
    return selectNode(nodeHeader, sNode).toElement().text().toInt();
}

float WWidget::selectNodeFloat(const QDomNode &nodeHeader, const QString sNode)
{
    return selectNode(nodeHeader, sNode).toElement().text().toFloat();
}

QString WWidget::selectNodeQString(const QDomNode &nodeHeader, const QString sNode)
{
    QString ret;
    QDomNode node = selectNode(nodeHeader, sNode);
    ret = node.toElement().text();
    return ret;
}

QDomNode WWidget::selectNode(const QDomNode &nodeHeader, const QString sNode)
{
    QDomNode node = nodeHeader.firstChild();
    while (!node.isNull())
    {
        if (node.nodeName() == sNode)
            return node;
            node = node.nextSibling();
    }
    return node;
}

const QString WWidget::getPath(QString location)
{
    QString l(location);
    return l.prepend(m_qPath);
}

void WWidget::setPixmapPath(QString qPath)
{
    m_qPath = qPath;
}



