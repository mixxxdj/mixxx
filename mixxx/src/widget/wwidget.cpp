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

#include <QtDebug>
#include <qtooltip.h>
#include "wwidget.h"
#include "controlobject.h"
#include "controlobjectthreadwidget.h"


// Static member variable definition
QString WWidget::m_qPath;
ConfigObject<ConfigValueKbd> *WWidget::m_spKbdConfigObject = 0;

WWidget::WWidget(QWidget * parent, const char * name, Qt::WFlags flags) : QWidget(parent,name, flags|Qt::WStaticContents|Qt::WNoAutoErase|Qt::WResizeNoErase)
{
    m_fValue = 0.;
    m_bOff = false;
    connect(this, SIGNAL(valueChangedLeftDown(double)), this, SLOT(slotReEmitValueDown(double)));
    connect(this, SIGNAL(valueChangedRightDown(double)), this, SLOT(slotReEmitValueDown(double)));
    connect(this, SIGNAL(valueChangedLeftUp(double)), this, SLOT(slotReEmitValueUp(double)));
    connect(this, SIGNAL(valueChangedRightUp(double)), this, SLOT(slotReEmitValueUp(double)));

	setFocusPolicy(Qt::ClickFocus);
    setBackgroundMode(Qt::NoBackground);
}

WWidget::~WWidget()
{
}

void WWidget::setKeyboardConfig(ConfigObject<ConfigValueKbd> * pKbdConfigObject)
{
    m_spKbdConfigObject = pKbdConfigObject;
}

void WWidget::setup(QDomNode node)
{
    // Set position
    QString pos = selectNodeQString(node, "Pos");
    int x = pos.left(pos.find(",")).toInt();
    int y = pos.mid(pos.find(",")+1).toInt();
    move(x,y);

    // Get tooltip
    QString strTooltip = selectNodeQString(node, "Tooltip");

    // For each connection
    QDomNode con = selectNode(node, "Connection");
    while (!con.isNull())
    {
        // Get ConfigKey
        QString key = selectNodeQString(con, "ConfigKey");

        ConfigKey configKey;
        configKey.group = key.left(key.find(","));
        configKey.item = key.mid(key.find(",")+1);

        // Check that the control exists
        ControlObject * control = ControlObject::getControl(configKey);
        if (control == NULL) {
            qWarning() << "Requested control does not exist:" << key;
            con = con.nextSibling();
            continue;
        }

        if (!selectNode(con, "OnOff").isNull() && selectNodeQString(con, "OnOff")=="true")
        {
            // Connect control proxy to widget
            (new ControlObjectThreadWidget(control))->setWidgetOnOff(this);
        }
        else
        {
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

            // Connect control proxy to widget
            (new ControlObjectThreadWidget(control))->setWidget(this, bEmitOnDownPress, state);

            // Add keyboard shortcut info to tooltip string
            QString shortcut = QString(" (%1)").arg(m_spKbdConfigObject->getValueString(configKey));
            if (!m_spKbdConfigObject->getValueString(configKey).isEmpty() && !strTooltip.contains(shortcut,false))
                strTooltip += shortcut;
        }
        con = con.nextSibling();
    }

    // Set tooltip if it exists
    if (strTooltip != "")
        QToolTip::add( this, strTooltip );

}

void WWidget::setValue(double fValue)
{
    m_fValue = fValue;
    update();
}

void WWidget::setOnOff(double d)
{
    if (d==0.)
        m_bOff = false;
    else
        m_bOff = true;

    repaint();
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
    QString text = selectNode(nodeHeader, sNode).toElement().text();
    bool ok;
    int conv = text.toInt(&ok, 0);
    if (ok) {
        return conv;
    } else {
        return 0;
    }
}

float WWidget::selectNodeFloat(const QDomNode &nodeHeader, const QString sNode)
{
    return selectNode(nodeHeader, sNode).toElement().text().toFloat();
}

QString WWidget::selectNodeQString(const QDomNode &nodeHeader, const QString sNode)
{
    QString ret;
    QDomNode node = selectNode(nodeHeader, sNode);
    if (!node.isNull())
        ret = node.toElement().text();
    else
        ret = "";
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

QDomElement WWidget::openXMLFile(QString path, QString name)
{
    QDomDocument doc(name);
    QFile file(path);
    if (!file.open(IO_ReadOnly))
    {
        qDebug() << "Could not open xml file:" << file.name();
        return QDomElement();
    }
    if (!doc.setContent(&file))
    {
        qWarning() << "Error parsing xml file:" << file.name();
        file.close();
        return QDomElement();
    }

    file.close();
    return doc.documentElement();
}

double WWidget::getValue() {
   return m_fValue;
}

void WWidget::updateValue(double fValue)
{
    setValue(fValue);
    m_fValue = fValue;
    emit(valueChangedUp(fValue));
    emit(valueChangedDown(fValue));
}
