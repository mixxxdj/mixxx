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
          m_iNoDigits(-1),
          m_dConstFactor(0.0) {
}

WNumber::~WNumber() {
}

void WNumber::setup(QDomNode node) {
    WLabel::setup(node);

    // Number of digits
    // TODO(rryan): This has been unused for a long time yet our skins specify
    // this value all over the place.
    m_iNoDigits = selectNodeInt(node, "NumberOfDigits");

    // Constant factor
    if (!selectNode(node, "ConstFactor").isNull()) {
        m_dConstFactor = selectNodeQString(node, "ConstFactor").toDouble();
    }

    setValue(0.);
}

void WNumber::setValue(double dValue) {
    double v = dValue + m_dConstFactor;
    int d1 = (int)floor((v-floor(v))*10.);
    int d2 = (int)floor((v-floor(v))*100.)%10;

    m_pLabel->setText(QString(m_qsText).append("%1.%2%3").arg(
        QString("%1").arg(static_cast<int>(v), 3, 10),
        QString("%1").arg(d1, 1, 10),
        QString("%1").arg(d2, 1, 10)));
}
