/***************************************************************************
                          wnumber.cpp  -  description
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

#include "wnumber.h"
#include <math.h>
#include <qfont.h>

WNumber::WNumber(QWidget *parent, const char *name ) : WWidget(parent,name)
{
    m_pLabel = new QLabel(parent);
    m_qsText = "";
}

WNumber::~WNumber()
{
    delete m_pLabel;
}

void WNumber::setup(QDomNode node)
{
    WWidget::setup(node);

    // Number of digits
    setNumDigits(selectNodeInt(node, "NumberOfDigits"));

    // Colors
    m_qBgColor.setNamedColor(WWidget::selectNodeQString(node, "BgColor"));
    m_pLabel->setPaletteBackgroundColor(m_qBgColor);
    m_qFgColor.setNamedColor(WWidget::selectNodeQString(node, "FgColor"));
    m_pLabel->setPaletteForegroundColor(m_qFgColor);

    // Text
    if (!selectNode(node, "Text").isNull())
        m_qsText = selectNodeQString(node, "Text");

    // Size
    QString size = selectNodeQString(node, "Size");
    int x = size.left(size.find(",")).toInt();
    int y = size.mid(size.find(",")+1).toInt();
    setFixedSize(x,y);
}

void WNumber::setFixedSize(int x,int y)
{
    WWidget::setFixedSize(x,y);
    m_pLabel->setFixedSize(x,y);
}

void WNumber::move(int x, int y)
{
    WWidget::move(x,y);
    m_pLabel->move(x,y);
}

void WNumber::setNumDigits(int n)
{
    m_iNoDigits = n;
}

void WNumber::setValue(double dValue)
{
    int d1 = (int)floor((dValue-floor(dValue))*10.);
    int d2 = (int)floor((dValue-floor(dValue))*100.)%10;

    m_pLabel->setText(QString(m_qsText).append("%1.%2%3").arg((int)dValue,3,10).arg(d1,1,10).arg(d2,1,10));
}


