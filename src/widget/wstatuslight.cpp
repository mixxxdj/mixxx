/***************************************************************************
                          wstatuslight.cpp  -  description
                             -------------------
    begin                : Wed May 30 2007
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
                           (C) 2007 by John Sully (converted from WVumeter)
    email                : jsully@scs.ryerson.ca
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "widget/wstatuslight.h"

#include <QPaintEvent>
#include <QStylePainter>
#include <QStyleOption>
#include <QtDebug>
#include <QPixmap>

WStatusLight::WStatusLight(QWidget * parent)
        : WWidget(parent),
          m_iPos(0) {
    setNoPos(0);
}

WStatusLight::~WStatusLight() {
}

void WStatusLight::setNoPos(int iNoPos) {
    // If pixmap array is already allocated, delete it.
    if (!m_pixmaps.empty()) {
        // Clear references to existing pixmaps.
        m_pixmaps.resize(0);
    }

    // values less than 2 make no sense (need at least off, on)
    if (iNoPos < 2) {
        iNoPos = 2;
    }
    m_pixmaps.resize(iNoPos);
}

void WStatusLight::setup(QDomNode node, const SkinContext& context) {
    // Number of states. Add one to account for the background.
    setNoPos(context.selectInt(node, "NumberPos") + 1);

    // Set pixmaps
    for (int i = 0; i < m_pixmaps.size(); ++i) {
        // Accept either PathStatusLight or PathStatusLight1 for value 1,
        QString nodeName = QString("PathStatusLight%1").arg(i);
        if (context.hasNode(node, nodeName)) {
            QDomElement statusLightNode = context.selectElement(node, nodeName);
            setPixmap(i, context.getPixmapSource(statusLightNode),
                      context.selectScaleMode(statusLightNode, Paintable::FIXED));
        } else if (i == 0 && context.hasNode(node, "PathBack")) {
            QDomElement statusLightNode = context.selectElement(node, "PathBack");
            setPixmap(i, context.getPixmapSource(statusLightNode),
                      context.selectScaleMode(statusLightNode, Paintable::FIXED));
        } else if (i == 1 && context.hasNode(node, "PathStatusLight")) {
            QDomElement statusLightNode = context.selectElement(node, "PathStatusLight");
            setPixmap(i, context.getPixmapSource(statusLightNode),
                      context.selectScaleMode(statusLightNode, Paintable::FIXED));
        } else {
            m_pixmaps[i].clear();
        }
    }
}

void WStatusLight::setPixmap(int iState, PixmapSource source,
                             Paintable::DrawMode mode) {
    if (iState < 0 || iState >= m_pixmaps.size()) {
        return;
    }

    PaintablePointer pPixmap = WPixmapStore::getPaintable(source, mode);
    if (!pPixmap.isNull() && !pPixmap->isNull()) {
        m_pixmaps[iState] = pPixmap;
        if (mode == Paintable::FIXED) {
            setFixedSize(pPixmap->size());
        }
    } else {
        qDebug() << "WStatusLight: Error loading pixmap:" << source.getPath() << iState;
        m_pixmaps[iState].clear();
    }
}

void WStatusLight::onConnectedControlChanged(double dParameter, double dValue) {
    // Enums are not currently represented using parameter space so it doesn't
    // make sense to use the parameter here yet.
    Q_UNUSED(dParameter);
    int newPos = static_cast<int>(dValue);

    if (m_pixmaps.size() == 2) {
        // original behavior for two-state lights: any non-zero value is "on"
        newPos = newPos > 0 ? 1 : 0;
    } else if (newPos < m_pixmaps.size() && newPos >= 0) {
        // multi-state behavior: values lie within the correct ranges
    } else {
        qDebug() << "Warning: wstatuslight asked for invalid position:"
                 << newPos << "max val:" << m_pixmaps.size()-1;
        return;
    }

    if (newPos != m_iPos) {
        m_iPos = newPos;
        update();
    }
}

void WStatusLight::paintEvent(QPaintEvent *) {
    QStyleOption option;
    option.initFrom(this);
    QStylePainter p(this);
    p.drawPrimitive(QStyle::PE_Widget, option);

    if (m_iPos < 0 || m_iPos >= m_pixmaps.size()) {
        return;
    }

    PaintablePointer pPixmap = m_pixmaps[m_iPos];

    if (pPixmap.isNull() || pPixmap->isNull()) {
        return;
    }

    pPixmap->draw(rect(), &p);
}
