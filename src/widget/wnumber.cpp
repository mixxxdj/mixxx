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

#include <math.h>
#include <QVBoxLayout>

#include "widget/wskincolor.h"

WNumber::WNumber(QWidget* pParent)
        : WLabel(pParent),
          m_iNoDigits(1),
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

void WNumber::onConnectedControlValueChanged(double v) {
    setValue(v);
}

void WNumber::setValue(double dValue) {
    double v = dValue + m_dConstFactor;

    setText(QString(m_qsText).append(QString::number(v, 'f', m_iNoDigits)));
}
