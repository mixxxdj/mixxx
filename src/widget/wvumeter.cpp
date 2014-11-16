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
#include "util/math.h"

#define DEFAULT_FALLTIME 20
#define DEFAULT_FALLSTEP 1
#define DEFAULT_HOLDTIME 400
#define DEFAULT_HOLDSIZE 5

WVuMeter::WVuMeter(QWidget* parent)
        : WWidget(parent),
          m_iPos(0),
          m_dParameter(0),
          m_iNoPos(0),
          m_bHorizontal(false),
          m_iPeakHoldSize(0),
          m_iPeakFallStep(0),
          m_iPeakHoldTime(0),
          m_iPeakFallTime(0),
          m_iPeakPos(0),
          m_dPeakParameter(0),
          m_iPeakHoldCountdown(0),
          m_iLastPos(0),
          m_iLastPeakPos(0) {
    m_timer.start();
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
        setPixmapBackground(context.getPixmapSource(context.selectNode(node, "PathBack")));
    }

    setPixmaps(context.getPixmapSource(context.selectNode(node, "PathVu")), bHorizontal);

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

void WVuMeter::setPixmapBackground(PixmapSource source) {
    m_pPixmapBack = WPixmapStore::getPaintable(source,
                                               Paintable::TILE);
    if (m_pPixmapBack.isNull() || m_pPixmapBack->isNull()) {
        qDebug() << metaObject()->className()
                 << "Error loading background pixmap:" << source.getPath();
    } else {
        setFixedSize(m_pPixmapBack->size());
    }
}

void WVuMeter::setPixmaps(PixmapSource source,
                          bool bHorizontal) {
    m_pPixmapVu = WPixmapStore::getPaintable(source,
                                             Paintable::STRETCH);
    if (m_pPixmapVu.isNull() || m_pPixmapVu->isNull()) {
        qDebug() << "WVuMeter: Error loading vu pixmap" << source.getPath();
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
    m_dParameter = dParameter;
    // Range check
    if (m_iPos > m_iNoPos) {
        m_iPos = m_iNoPos;
    } else if (m_iPos < 0) {
        m_iPos = 0;
    }

    if (dParameter > 0.) {
        setPeak(m_iPos, dParameter);
    } else {
        // A 0.0 value is very unlikely except when the VU Meter is disabled
        m_iPeakPos = 0;
        m_dPeakParameter = 0;
    }

    double msecsElapsed = m_timer.restart() / 1000000.0;
    updateState(msecsElapsed);
}

void WVuMeter::setPeak(int pos, double parameter) {
    if (pos > m_iPeakPos) {
        m_iPeakPos = pos;
        m_dPeakParameter = parameter;
        m_iPeakHoldCountdown = m_iPeakHoldTime;
    }
}

void WVuMeter::updateState(double msecsElapsed) {
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
    m_dPeakParameter -= static_cast<double>(m_iPeakFallStep) *
            msecsElapsed /
            static_cast<double>(m_iPeakFallTime * m_iNoPos);
    m_dPeakParameter = math_clamp_fast(m_dPeakParameter, 0.0, 1.0);
    m_iPeakPos = math_clamp(static_cast<int>(m_dPeakParameter * m_iNoPos),
                            0, m_iNoPos);
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
        int widgetWidth = width();
        int widgetHeight = height();

        // Draw (part of) vu
        if (m_bHorizontal) {
            // This is a hack to fix something weird with horizontal VU meters:
            if (m_iPos == 0)
                m_iPos = 1;

            int widgetPosition = math_clamp(
                static_cast<int>(widgetWidth * m_dParameter), 0, widgetWidth);

            QRectF targetRect(0, 0, widgetPosition, widgetHeight);
            QRectF sourceRect(0, 0, m_iPos,  m_pPixmapVu->height());
            m_pPixmapVu->draw(targetRect, &p, sourceRect);

            if(m_iPeakHoldSize > 0 && m_iPeakPos > 0) {
                int widgetPeakPosition = math_clamp(
                    static_cast<int>(widgetWidth * m_dPeakParameter),
                    0, widgetWidth);
                int widgetPeakHoldSize = widgetWidth * static_cast<double>(m_iPeakHoldSize) /
                        static_cast<double>(m_iNoPos);
                targetRect = QRectF(widgetPeakPosition - m_iPeakHoldSize, 0,
                                    widgetPeakHoldSize, widgetHeight);
                sourceRect = QRectF(m_pPixmapVu->width() - m_iPeakPos, 0,
                                    m_iPeakHoldSize, m_pPixmapVu->height());
                m_pPixmapVu->draw(targetRect, &p, sourceRect);
            }
        } else {
            int widgetPosition = math_clamp(
                static_cast<int>(widgetHeight * m_dParameter), 0, widgetHeight);
            QRectF targetRect(0, widgetHeight - widgetPosition,
                              widgetWidth, widgetPosition);
            QRectF sourceRect(0, m_iNoPos - m_iPos, m_pPixmapVu->width(),
                              m_iPos);
            m_pPixmapVu->draw(targetRect, &p, sourceRect);

            if (m_iPeakHoldSize > 0 && m_iPeakPos > 0) {
                int widgetPeakPosition = math_clamp(
                    static_cast<int>(widgetHeight * m_dPeakParameter),
                    0, widgetHeight);
                int widgetPeakHoldSize = widgetHeight * static_cast<double>(m_iPeakHoldSize) /
                        static_cast<double>(m_iNoPos);
                targetRect = QRectF(0, widgetHeight - widgetPeakPosition,
                                    widgetWidth, widgetPeakHoldSize);
                sourceRect = QRectF(0, m_iNoPos - m_iPeakPos,
                                    m_pPixmapVu->width(), m_iPeakHoldSize);
                m_pPixmapVu->draw(targetRect, &p, sourceRect);
            }
        }
    }
    m_iLastPos = m_iPos;
    m_iLastPeakPos = m_iPeakPos;
}
