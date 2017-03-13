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

#include "widget/wskincolor.h"

WLabel::WLabel(QWidget* pParent)
        : QLabel(pParent),
          WBaseWidget(this),
          m_qsText("") {
}

WLabel::~WLabel() {
}

void WLabel::setup(QDomNode node, const SkinContext& context) {
    // Colors
    QPalette pal = palette(); //we have to copy out the palette to edit it since it's const (probably for threadsafety)
    if (context.hasNode(node, "BgColor")) {
        m_qBgColor.setNamedColor(context.selectString(node, "BgColor"));
        pal.setColor(this->backgroundRole(), WSkinColor::getCorrectColor(m_qBgColor));
        setAutoFillBackground(true);
    }
    m_qFgColor.setNamedColor(context.selectString(node, "FgColor"));
    pal.setColor(this->foregroundRole(), WSkinColor::getCorrectColor(m_qFgColor));
    setPalette(pal);

    // Text
    if (context.hasNode(node, "Text"))
        m_qsText = context.selectString(node, "Text");
    setText(m_qsText);

    // Font size
    if (context.hasNode(node, "FontSize")) {
        int fontsize = context.selectString(node, "FontSize").toInt();
        setFont(QFont("Helvetica", fontsize, QFont::Normal));
    }

    // Alignment
    if (context.hasNode(node, "Alignment")) {
        if (context.selectString(node, "Alignment").toLower() == "right") {
            setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        } else if (context.selectString(node, "Alignment").toLower() == "center") {
            setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        } else if (context.selectString(node, "Alignment").toLower() == "left") {
            setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }
}

bool WLabel::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QLabel::event(pEvent);
}

void WLabel::fillDebugTooltip(QStringList* debug) {
    WBaseWidget::fillDebugTooltip(debug);
    *debug << QString("Text: \"%1\"").arg(text());
}
