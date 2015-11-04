/***************************************************************************
                          wvumeter.cpp  -  description
                             -------------------
    begin                : Fri Jul 22 2003
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

#include "widget/wvumeter.h"

#include <QStylePainter>
#include <QStyleOption>
#include <QPaintEvent>
#include <QtDebug>
#include <QPixmap>

#include "widget/wpixmapstore.h"
#include "util/timer.h"

#define DEFAULT_FALLTIME 20
#define DEFAULT_FALLSTEP 1
#define DEFAULT_HOLDTIME 400
#define DEFAULT_HOLDSIZE 5

WVuMeter::WVuMeter(QWidget* parent) :
        WWidget(parent),
        m_iPos(0),
        m_iNoPos(0),
        m_bHorizontal(false),
        m_iPeakHoldSize(0),
        m_iPeakFallStep(0),
        m_iPeakHoldTime(0),
        m_iPeakFallTime(0),
        m_iPeakPos(0),
        m_iPeakHoldCountdown(0),
        m_iLastPos(0),
        m_iLastPeakPos(0) {
}

WVuMeter::~WVuMeter() {
    resetPositions();
}

void WVuMeter::setup(QDomNode node, const SkinContext& context) {
    // Set pixmaps
    bool bHorizontal = context.hasNode(node, "Horizontal") &&
            context.selectString(node, "Horizontal") == "true";

    // Set background pixmap if available
    if (context.hasNode(node, "PathBack")) {
        setPixmapBackground(context.getPixmapPath(context.selectNode(node, "PathBack")));
    }

    setPixmaps(context.getPixmapPath(context.selectNode(node, "PathVu")), bHorizontal);

    m_iPeakHoldSize = context.selectInt(node, "PeakHoldSize");
    if (m_iPeakHoldSize < 0 || m_iPeakHoldSize > 100)
        m_iPeakHoldSize = DEFAULT_HOLDSIZE;

    m_iPeakFallStep = context.selectInt(node, "PeakFallStep");
    if (m_iPeakFallStep < 1 || m_iPeakFallStep > 1000)
        m_iPeakFallStep = DEFAULT_FALLSTEP;

    m_iPeakHoldTime = context.selectInt(node, "PeakHoldTime");
    if (m_iPeakHoldTime < 1 || m_iPeakHoldTime > 3000)
        m_iPeakHoldTime = DEFAULT_HOLDTIME;

    m_iPeakFallTime = context.selectInt(node, "PeakFallTime");
    if (m_iPeakFallTime < 1 || m_iPeakFallTime > 1000)
        m_iPeakFallTime = DEFAULT_FALLTIME;
}

void WVuMeter::resetPositions() {
    m_pPixmapBack.clear();
    m_pPixmapVu.clear();
}

void WVuMeter::setPixmapBackground(const QString& filename) {
    m_pPixmapBack = WPixmapStore::getPaintable(filename,
                                               Paintable::TILE);
    if (m_pPixmapBack.isNull() || m_pPixmapBack->isNull()) {
        qDebug() << metaObject()->className()
                 << "Error loading background pixmap:" << filename;
    } else {
        setFixedSize(m_pPixmapBack->size());
    }
}

void WVuMeter::setPixmaps(const QString &vuFilename,
                          bool bHorizontal) {
    m_pPixmapVu = WPixmapStore::getPaintable(vuFilename,
                                             Paintable::STRETCH);
    if (m_pPixmapVu.isNull() || m_pPixmapVu->isNull()) {
        qDebug() << "WVuMeter: Error loading vu pixmap" << vuFilename;
    } else {
        m_bHorizontal = bHorizontal;
        if (m_bHorizontal) {
            m_iNoPos = m_pPixmapVu->width();
        } else {
            m_iNoPos = m_pPixmapVu->height();
        }
    }
}

void WVuMeter::onConnectedControlChanged(double dParameter, double dValue) {
    Q_UNUSED(dValue);
    m_iPos = static_cast<int>(dParameter * m_iNoPos);
    // Range check
    if (m_iPos > m_iNoPos) {
        m_iPos = m_iNoPos;
    } else if (m_iPos < 0) {
        m_iPos = 0;
    }

    if (dParameter > 0.) {
        setPeak(m_iPos);
    } else {
        // A 0.0 value is very unlikely except when the VU Meter is disabled
        m_iPeakPos = 0;
    }

    // TODO: use something much more lightweight than currentTime.
    QTime currentTime = QTime::currentTime();
    int msecsElapsed = m_lastUpdate.msecsTo(currentTime);
    m_lastUpdate = currentTime;
    updateState(msecsElapsed);
}

void WVuMeter::setPeak(int pos) {
    if (pos > m_iPeakPos) {
        m_iPeakPos = pos;
        m_iPeakHoldCountdown = m_iPeakHoldTime;
    }
}

void WVuMeter::updateState(int msecsElapsed) {
    // If we're holding at a peak then don't update anything
    m_iPeakHoldCountdown -= msecsElapsed;
    if (m_iPeakHoldCountdown > 0) {
        return;
    } else {
        m_iPeakHoldCountdown = 0;
    }

    // Otherwise, decrement the peak position by the fall step size times the
    // milliseconds elapsed over the fall time multiplier. The peak will fall
    // FallStep times (out of 128 steps) every FallTime milliseconds.

    m_iPeakPos -= float(m_iPeakFallStep) * float(msecsElapsed) / float(m_iPeakFallTime);
    if (m_iPeakPos < 0)
        m_iPeakPos = 0;
}

void WVuMeter::maybeUpdate() {
    if (m_iPos != m_iLastPos || m_iPeakPos != m_iLastPeakPos) {
        repaint();
    }
}

void WVuMeter::paintEvent(QPaintEvent *) {
    ScopedTimer t("WVuMeter::paintEvent");

    QStyleOption option;
    option.initFrom(this);
    QStylePainter p(this);
    p.drawPrimitive(QStyle::PE_Widget, option);

    if (!m_pPixmapBack.isNull() && !m_pPixmapBack->isNull()) {
        // Draw background.
        m_pPixmapBack->draw(0, 0, &p);
    }

    if (!m_pPixmapVu.isNull() && !m_pPixmapVu->isNull()) {
        // Draw (part of) vu
        if (m_bHorizontal) {
            // This is a hack to fix something weird with horizontal VU meters:
            if (m_iPos == 0)
                m_iPos = 1;

            QPointF targetPoint(0, 0);
            QRectF sourceRect(0, 0, m_iPos, m_pPixmapVu->height());
            m_pPixmapVu->draw(targetPoint, &p, sourceRect);

            if(m_iPeakHoldSize > 0 && m_iPeakPos > 0) {
                targetPoint = QPointF(m_iPeakPos - m_iPeakHoldSize, 0);
                sourceRect = QRectF(m_iPeakPos - m_iPeakHoldSize, 0,
                                    m_iPeakHoldSize, m_pPixmapVu->height());
                m_pPixmapVu->draw(targetPoint, &p, sourceRect);
            }
        } else {
            QPointF targetPoint(0, m_iNoPos - m_iPos);
            QRectF sourceRect(0, m_iNoPos - m_iPos, m_pPixmapVu->width(), m_iPos);
            m_pPixmapVu->draw(targetPoint, &p, sourceRect);

            if (m_iPeakHoldSize > 0 && m_iPeakPos > 0) {
                targetPoint = QPointF(0, m_pPixmapVu->height() - m_iPeakPos);
                sourceRect = QRectF(0, m_pPixmapVu->height() - m_iPeakPos,
                                    m_pPixmapVu->width(), m_iPeakHoldSize);
                m_pPixmapVu->draw(targetPoint, &p, sourceRect);
            }
        }
    }
    m_iLastPos = m_iPos;
    m_iLastPeakPos = m_iPeakPos;
}
