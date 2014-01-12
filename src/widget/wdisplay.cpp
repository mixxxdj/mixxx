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

void WDisplay::setup(QDomNode node, const SkinContext& context) {
    // Set background pixmap if available
    if (context.hasNode(node, "BackPath")) {
        setPixmapBackground(context.getSkinPath(
            context.selectString(node, "BackPath")));
    }

    // Number of states
    setPositions(context.selectInt(node, "NumberStates"));


    // Load knob pixmaps
    QString path = context.selectString(node, "Path");
    for (int i = 0; i < m_pixmaps.size(); ++i) {
        setPixmap(&m_pixmaps, i, context.getSkinPath(path.arg(i)));
    }

    // See if disabled images is defined, and load them...
    if (context.hasNode(node, "DisabledPath")) {
        QString disabledPath = context.selectString(node, "DisabledPath");
        for (int i = 0; i < m_disabledPixmaps.size(); ++i) {
            setPixmap(&m_disabledPixmaps, i,
                      context.getSkinPath(disabledPath.arg(i)));
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
    // When there are an even number of pixmaps by convention we want a value of
    // 0.5 to align to the lower of the two middle pixmaps. In Mixxx < 1.12.0 we
    // accomplished this by the below formula:
    // index = (m_value - 64.0/127.0) * (numPixmaps() - 1) + numPixmaps() / 2.0;

    // But it's just as good to use m_value * numPixmaps() - epsilon. Using
    // numPixmaps() instead of numPixmaps() - 1 ensures that every pixmap shares
    // an equal slice of the value. Using m_value * (numPixmaps() - 1) gives an
    // unequal slice of the value to the last pixmaps.

    // Example:
    // 3 pixmaps
    // m_value * numPixmaps()
    // idx: 0       1       2       3
    // val: 0.0 ... 0.3 ... 0.6 ... 1.0
    // Even distribution of value range, value 1 is out of bounds (3).

    // m_value * (numPixmaps() - 1)
    // idx: 0       1       2
    // val: 0.0 ... 0.5 ... 1.0
    // Pixmap 2 is only shown at value 1.

    // floor(m_value * (numPixmaps() - 1) + 0.5)
    // idx: 0       1        2
    // val: 0.0 ... 0.25 ... 0.75 ... 1.0
    // Pixmap 0 and Pixmap 2 only shown for 0.25 of value range

    // 4 pixmaps
    // m_value * numPixmaps()
    // idx: 0       1        2       3        4
    // val: 0.0 ... 0.25 ... 0.5 ... 0.75 ... 1.0
    // Even distribution of value range, value 1 is out of bounds (4).

    // Subtracting an epsilon prevents out of bound values at the end of the
    // range and biases the middle value towards the lower of the 2 center
    // pixmaps when there are an even number of pixmaps.
    return static_cast<int>(getControlParameterDisplay() * numPixmaps() - 0.00001);
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
    const QVector<PaintablePointer>& pixmaps = (!isEnabled() && m_bDisabledLoaded) ?
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
