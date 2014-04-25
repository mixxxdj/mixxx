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

#include "widget/wnumber.h"

WNumber::WNumber(QWidget* pParent)
        : WLabel(pParent),
          m_iNoDigits(2),
          m_dConstFactor(0.0) {
}

WNumber::~WNumber() {
}

void WNumber::setup(QDomNode node, const SkinContext& context) {
    WLabel::setup(node, context);

    // Number of digits after the decimal.
    if (context.hasNode(node, "NumberOfDigits")) {
        m_iNoDigits = context.selectInt(node, "NumberOfDigits");
    }

    // Constant factor
    if (context.hasNode(node, "ConstFactor")) {
        m_dConstFactor = context.selectString(node, "ConstFactor").toDouble();
    }

    setValue(0.);
}

void WNumber::onConnectedControlChanged(double dParameter, double dValue) {
    Q_UNUSED(dParameter);
    // We show the actual control value instead of its parameter.
    setValue(dValue);
}

void WNumber::setValue(double dValue) {
    double v = dValue + m_dConstFactor;

    if (m_qsText.contains("%1")) {
        setText(m_qsText.arg(QString::number(v, 'f', m_iNoDigits)));
    } else {
        setText(m_qsText + QString::number(v, 'f', m_iNoDigits));
    }
}
