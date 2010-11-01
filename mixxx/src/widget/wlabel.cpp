/***************************************************************************
                          wlabel.cpp  -  description
                             -------------------
    begin                : Wed Jan 5 2005
    copyright            : (C) 2003 by Tue Haste Andersen
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

#include "wlabel.h"
#include "wskincolor.h"
#include <math.h>
#include <qfont.h>
//Added by qt3to4:
#include <QLabel>

WLabel::WLabel(QWidget * parent) : WWidget(parent)
{
    m_pLabel = new QLabel(this);
    m_qsText = "";
}

WLabel::~WLabel()
{
    delete m_pLabel;
}

void WLabel::setup(QDomNode node)
{
    // Colors
    QPalette palette = m_pLabel->palette(); //we have to copy out the palette to edit it since it's const (probably for threadsafety)

    if(!WWidget::selectNode(node, "BgColor").isNull()) {
        m_qBgColor.setNamedColor(WWidget::selectNodeQString(node, "BgColor"));
        //m_pLabel->setPaletteBackgroundColor(WSkinColor::getCorrectColor(m_qBgColor)); //deprecated
        palette.setColor(this->backgroundRole(), WSkinColor::getCorrectColor(m_qBgColor));
        m_pLabel->setAutoFillBackground(true);
    }
    m_qFgColor.setNamedColor(WWidget::selectNodeQString(node, "FgColor"));
    //m_pLabel->setPaletteForegroundColor(WSkinColor::getCorrectColor(m_qFgColor)); //deprecated
    palette.setColor(this->foregroundRole(), WSkinColor::getCorrectColor(m_qFgColor));

    m_pLabel->setPalette(palette);

    m_pLabel->setToolTip(toolTip());

    // Text
    if (!selectNode(node, "Text").isNull())
        m_qsText = selectNodeQString(node, "Text");
    m_pLabel->setText(m_qsText);

    // Size
    QString size = selectNodeQString(node, "Size");
    int x = size.left(size.indexOf(",")).toInt();
    int y = size.mid(size.indexOf(",")+1).toInt();
    m_pLabel->setFixedSize(x,y);

    // Alignment
    if (!selectNode(node, "Alignment").isNull())
    {
        if (selectNodeQString(node, "Alignment")=="right")
            m_pLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    }

    QString style = selectNodeQString(node, "Style");
    if (style != "") {
        m_pLabel->setStyleSheet(style);
    }
}

void WLabel::setAlignment(Qt::Alignment i)
{
    m_pLabel->setAlignment(i);
}
