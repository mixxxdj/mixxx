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

#include <QStylePainter>
#include <QStyleOption>
#include <QPaintEvent>
#include <QtDebug>
#include <QPixmap>

#include "widget/wpixmapstore.h"

WDisplay::WDisplay(QWidget * parent)
        : WWidget(parent),
          m_pPixmapBack(NULL),
          m_bDisabledLoaded(false) {
    setPositions(0);
}

WDisplay::~WDisplay() {
    resetPositions();
}

void WDisplay::setup(QDomNode node) {
    // Set background pixmap if available
    if (!selectNode(node, "BackPath").isNull()) {
        setPixmapBackground(getPath(selectNodeQString(node, "BackPath")));
    }

    // Number of states
    setPositions(selectNodeInt(node, "NumberStates"));


    // Load knob pixmaps
    QString path = selectNodeQString(node, "Path");
    for (int i = 0; i < m_pixmaps.size(); ++i) {
        setPixmap(&m_pixmaps, i, getPath(path.arg(i)));
    }

    // See if disabled images is defined, and load them...
    if (!selectNode(node, "DisabledPath").isNull()) {
        QString disabledPath = selectNodeQString(node, "DisabledPath");
        for (int i = 0; i < m_disabledPixmaps.size(); ++i) {
            setPixmap(&m_disabledPixmaps, i,
                      getPath(disabledPath.arg(i)));
        }
        m_bDisabledLoaded = true;
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
    m_disabledPixmaps.resize(iNoPos);
}

void WDisplay::resetPositions() {
    m_pPixmapBack.clear();
    m_pixmaps.resize(0);
    m_disabledPixmaps.resize(0);
}

void WDisplay::setPixmapBackground(const QString& filename) {
    m_pPixmapBack = WPixmapStore::getPaintable(filename);
    if (m_pPixmapBack.isNull() || m_pPixmapBack->isNull()) {
        qDebug() << metaObject()->className()
                 << "Error loading background pixmap:" << filename;
    }
}

void WDisplay::setPixmap(QVector<PaintablePointer>* pPixmaps, int iPos,
                         const QString& filename) {
    if (iPos < 0 || iPos >= pPixmaps->size()) {
        return;
    }

    PaintablePointer pPixmap = WPixmapStore::getPaintable(filename);

    if (pPixmap.isNull() || pPixmap->isNull()) {
        qDebug() << metaObject()->className()
                 << "Error loading pixmap:" << filename;
    } else {
        (*pPixmaps)[iPos] = pPixmap;
        setFixedSize(pPixmap->size());
    }
}

int WDisplay::getActivePixmapIndex() const {
    return static_cast<int>(m_value * m_pixmaps.size());
}

void WDisplay::paintEvent(QPaintEvent* ) {
    QStyleOption option;
    option.initFrom(this);
    QStylePainter p(this);
    p.drawPrimitive(QStyle::PE_Widget, option);

    if (m_pPixmapBack) {
        m_pPixmapBack->draw(0, 0, &p);
    }

    // If we are disabled, use the disabled pixmaps. If not, use the regular
    // pixmaps.
    const QVector<PaintablePointer>& pixmaps = (m_bOff && m_bDisabledLoaded) ?
            m_disabledPixmaps : m_pixmaps;

    if (pixmaps.empty()) {
        return;
    }

    int idx = getActivePixmapIndex();

    // Clamp active pixmap index to valid ranges.
    if (idx < 0) {
        idx = 0;
    } else if (idx >= pixmaps.size()) {
        idx = pixmaps.size() - 1;
    }

    PaintablePointer pPixmap = pixmaps[idx];
    if (pPixmap) {
        pPixmap->draw(0, 0, &p);
    }
}
