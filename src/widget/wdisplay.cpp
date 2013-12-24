/***************************************************************************
                          wdisplay.cpp  -  description
                             -------------------
    begin                : Fri Jun 21 2002
    copyright            : (C) 2002 by Tue & Ken Haste Andersen
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

#include "widget/wdisplay.h"

#include <QPainter>
#include <QPaintEvent>
#include <QtDebug>
#include <QPixmap>

#include "widget/wpixmapstore.h"

WDisplay::WDisplay(QWidget * parent)
        : WWidget(parent) {
    setPositions(0);
}

WDisplay::~WDisplay() {
    resetPositions();
}

void WDisplay::setup(QDomNode node) {
    // Number of states
    setPositions(selectNodeInt(node, "NumberStates"));

    // Load knob pixmaps
    QString path = selectNodeQString(node, "Path");
    for (int i = 0; i < m_pixmaps.size(); ++i) {
        setPixmap(i, getPath(path.arg(i)));
    }
}

void WDisplay::setPositions(int iNoPos) {
    resetPositions();

    if (iNoPos < 0) {
        qWarning() << "Negative NumberStates for Display.";
        iNoPos = 0;
    }

    // QVector inserts NULLs for the new pixmaps.
    m_pixmaps.resize(iNoPos);
}

void WDisplay::resetPositions() {
    for (int i = 0; i < m_pixmaps.size(); ++i) {
        QPixmap* pPixmap = m_pixmaps[i];
        if (pPixmap) {
            WPixmapStore::deletePixmap(pPixmap);
        }
    }
    m_pixmaps.resize(0);
}

void WDisplay::setPixmap(int iPos, const QString& filename) {
    if (iPos < 0 || iPos >= m_pixmaps.size()) {
        return;
    }
    m_pixmaps[iPos] = WPixmapStore::getPixmap(filename);
    if (!m_pixmaps[iPos])
        qDebug() << "WDisplay: Error loading pixmap" << filename;
    else
        setFixedSize(m_pixmaps[iPos]->size());
}

void WDisplay::paintEvent(QPaintEvent* ) {
    int idx = static_cast<int>(
        m_value * static_cast<double>(m_pixmaps.size()) / 128.0);

    if (m_pixmaps.empty()) {
        return;
    }

    if (idx < 0) {
        idx = 0;
    } else if (idx >= m_pixmaps.size()) {
        idx = m_pixmaps.size() - 1;
    }

    QPixmap* pPixmap = m_pixmaps[idx];
    if (pPixmap) {
        QPainter p(this);
        p.drawPixmap(0, 0, *pPixmap);
    }
}
