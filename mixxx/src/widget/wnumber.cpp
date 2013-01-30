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
#include "wskincolor.h"
#include <math.h>
#include <qfont.h>
//Added by qt3to4:
#include <QLabel>

WNumber::WNumber(QWidget * parent) : WWidget(parent)
{
    m_pLabel = new QLabel(this);
    QLayout* pLayout = new QVBoxLayout(this);
    pLayout->setContentsMargins(0, 0, 0, 0);
    pLayout->addWidget(m_pLabel);
    setLayout(pLayout);
    m_qsText = "";
    m_dConstFactor = 0.;
}

WNumber::~WNumber()
{
    delete m_pLabel;
}

void WNumber::setup(QDomNode node)
{
    // Number of digits
    setNumDigits(selectNodeInt(node, "NumberOfDigits"));

    // Colors
    QPalette palette = m_pLabel->palette(); //we have to copy out the palette to edit it since it's const (probably for threadsafety)
    if (!WWidget::selectNode(node, "BgColor").isNull()) {
        m_qBgColor.setNamedColor(WWidget::selectNodeQString(node, "BgColor"));
        palette.setColor(this->backgroundRole(), WSkinColor::getCorrectColor(m_qBgColor));
        m_pLabel->setAutoFillBackground(true);
    }
    m_qFgColor.setNamedColor(WWidget::selectNodeQString(node, "FgColor"));
    palette.setColor(this->foregroundRole(), WSkinColor::getCorrectColor(m_qFgColor));
    m_pLabel->setPalette(palette);

    // Text
    if (!selectNode(node, "Text").isNull())
        m_qsText = selectNodeQString(node, "Text");

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

    // Constant factor
    if (!selectNode(node, "ConstFactor").isNull())
    {
        m_dConstFactor = selectNodeQString(node, "ConstFactor").toDouble();
        setValue(0.);
    }
}

void WNumber::setNumDigits(int n)
{
    m_iNoDigits = n;
}

void WNumber::setValue(double dValue)
{
    double v = dValue+m_dConstFactor;
    int d1 = (int)floor((v-floor(v))*10.);
    int d2 = (int)floor((v-floor(v))*100.)%10;

    m_pLabel->setText(QString(m_qsText).append("%1.%2%3").arg(
        QString("%1").arg(static_cast<int>(v), 3, 10),
        QString("%1").arg(d1, 1, 10),
        QString("%1").arg(d2, 1, 10)));
}

void WNumber::setConstFactor(double c)
{
    m_dConstFactor = c;
}
