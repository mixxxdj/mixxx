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

#include "widget/wlabel.h"

#include <QFont>
#include <QVBoxLayout>

#include "widget/wskincolor.h"

WLabel::WLabel(QWidget* pParent)
        : WWidget(pParent),
          m_pLabel(new QLabel(this)),
          m_qsText("") {
    QLayout* pLayout = new QVBoxLayout(this);
    pLayout->setContentsMargins(0, 0, 0, 0);
    pLayout->addWidget(m_pLabel);
    setLayout(pLayout);
}

WLabel::~WLabel() {
    delete m_pLabel;
}

void WLabel::setup(QDomNode node) {
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
    m_pLabel->setText(m_qsText);

    // Font size
    if (!selectNode(node, "FontSize").isNull()) {
        int fontsize = 9;
        fontsize = selectNodeQString(node, "FontSize").toInt();
        m_pLabel->setFont( QFont("Helvetica",fontsize,QFont::Normal) );
    }

    // Alignment
    if (!selectNode(node, "Alignment").isNull()) {
        if (selectNodeQString(node, "Alignment")=="right") {
            m_pLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        } else if (selectNodeQString(node, "Alignment")=="center") {
            m_pLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        }
    }
}
