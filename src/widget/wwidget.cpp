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

#include "widget/wwidget.h"

WWidget::WWidget(QWidget* parent, Qt::WindowFlags flags)
        : QWidget(parent, flags),
          WBaseWidget(this) {
    setAttribute(Qt::WA_StaticContents);
    setFocusPolicy(Qt::ClickFocus);
}

WWidget::~WWidget() {
}

void WWidget::onConnectedControlValueChanged(double value) {
    Q_UNUSED(value);
    update();
}

bool WWidget::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QWidget::event(pEvent);
}
