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

#include "wkey.h"
#include "wskincolor.h"
#include <math.h>
#include <qfont.h>
//Added by qt3to4:
#include <QLabel>

WKey::WKey(QWidget * parent) : WWidget(parent)
{
    m_pLabel = new QLabel(this);
    m_qsText = "";
   // m_dConstFactor = 0.;
    //qDebug()<<"key 3";
}

WKey::~WKey()
{
    delete m_pLabel;
}

void WKey::setup(QDomNode node)
{
    // Number of digits
    //setNumDigits(selectNodeInt(node, "NumberOfDigits"));

    // Colors
    QPalette palette = m_pLabel->palette(); //we have to copy out the palette to edit it since it's const (probably for threadsafety)

    if(!WWidget::selectNode(node, "BgColor").isNull()) {
        m_qBgColor.setNamedColor(WWidget::selectNodeQString(node, "BgColor"));
        //m_pLabel->setPaletteBackgroundColor(WSkinColor::getCorrectColor(m_qBgColor));
        palette.setColor(this->backgroundRole(), WSkinColor::getCorrectColor(m_qBgColor));
        m_pLabel->setAutoFillBackground(true);
    }
    m_qFgColor.setNamedColor(WWidget::selectNodeQString(node, "FgColor"));
    //m_pLabel->setPaletteForegroundColor(WSkinColor::getCorrectColor(m_qFgColor));
    palette.setColor(this->foregroundRole(), WSkinColor::getCorrectColor(m_qFgColor));

    m_pLabel->setPalette(palette);

    m_pLabel->setToolTip(toolTip());

    // Text
    if (!selectNode(node, "Text").isNull())
        m_qsText = selectNodeQString(node, "Text");

    QString size = selectNodeQString(node, "Size");
    int x = size.left(size.indexOf(",")).toInt();
    int y = size.mid(size.indexOf(",")+1).toInt();
    m_pLabel->setFixedSize(x,y);

    // FWI: Begin of font size patch
    if (!selectNode(node, "FontSize").isNull()) {
        int fontsize = 9;
        fontsize = selectNodeQString(node, "FontSize").toInt();
        m_pLabel->setFont( QFont("Helvetica",fontsize,QFont::Normal) );
    }
    // FWI: End of font size patch

    // Alignment
    if (!selectNode(node, "Alignment").isNull())
    {
        if (selectNodeQString(node, "Alignment")=="right")
            m_pLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        // FWI: Begin of font alignment patch
        else if (selectNodeQString(node, "Alignment")=="center")
            m_pLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        // FWI: End of font alignment patch
    }
   // qDebug()<<"key 4";
    // Constant factor
    /*if (!selectNode(node, "ConstFactor").isNull())
    {
        m_dConstFactor = selectNodeQString(node, "ConstFactor").toDouble();
        setValue(0.);
    }*/
    setValue(0);
}

/*void WNumber::setNumDigits(int n)
{
    m_iNoDigits = n;
}*/

void WKey::setValue(double dValue)
{
    /*double v = dValue+m_dConstFactor;
    int d1 = (int)floor((v-floor(v))*10.);
    int d2 = (int)floor((v-floor(v))*100.)%10;*/
    //qDebug()<<"key 1";
    qDebug()<<"key"<<dValue;

    m_pLabel->setText(convertKey(dValue));
    //qDebug()<<"key 2";
    /*m_pLabel->setText(QString(m_qsText).append("%1.%2%3").arg(
        QString("%1").arg(static_cast<int>(v), 3, 10),
        QString("%1").arg(d1, 1, 10),
        QString("%1").arg(d2, 1, 10)));*/
}

QString WKey::convertKey(double dValue)
{
    QString key="";
    if(dValue > 24 || dValue<=0)key="err";
    else if(dValue == 1)key = "C";
    else if(dValue == 2)key = "C#";
    else if(dValue == 3)key = "D";
    else if(dValue == 4)key = "D#";
    else if(dValue == 5)key = "E";
    else if(dValue == 6)key = "F";
    else if(dValue == 7)key = "F#";
    else if(dValue == 8)key = "G";
    else if(dValue == 9)key = "G#";
    else if(dValue == 10)key = "A";
    else if(dValue == 11)key = "A#";
    else if(dValue == 12)key = "B";
    else if(dValue == 13)key = "c";
    else if(dValue == 14)key = "c#";
    else if(dValue == 15)key = "d";
    else if(dValue == 16)key = "d#";
    else if(dValue == 17)key = "e";
    else if(dValue == 18)key = "f";
    else if(dValue == 19)key = "f#";
    else if(dValue == 20)key = "g";
    else if(dValue == 21)key = "g#";
    else if(dValue == 22)key = "a";
    else if(dValue == 23)key = "a#";
    else if(dValue == 24)key = "b";
    return key;
}

/*void WNumber::setConstFactor(double c)
{
    m_dConstFactor = c;
}*/
