#include "widget/wvumeter.h"

#include <QPaintEvent>
#include <QPixmap>
#include <QStyleOption>
#include <QStylePainter>
#include <QtDebug>

#include "moc_wvumeter.cpp"
#include "util/math.h"
#include "util/timer.h"
#include "widget/wpixmapstore.h"

#define DEFAULT_FALLTIME 20
#define DEFAULT_FALLSTEP 1
#define DEFAULT_HOLDTIME 400
#define DEFAULT_HOLDSIZE 5

WVuMeter::WVuMeter(QWidget* parent)
        : WWidget(parent),
          m_dParameter(0),
          m_dPeakParameter(0),
          m_dLastParameter(0),
          m_dLastPeakParameter(0),
          m_iPixmapLength(0),
          m_bHorizontal(false),
          m_iPeakHoldSize(0),
          m_iPeakFallStep(0),
          m_iPeakHoldTime(0),
          m_iPeakFallTime(0),
          m_dPeakHoldCountdownMs(0) {
    m_timer.start();
}

void WVuMeter::setup(const QDomNode& node, const SkinContext& context) {
    // Set pixmaps
    bool bHorizontal = false;
    (void)context.hasNodeSelectBool(node, "Horizontal", &bHorizontal);

    // Set background pixmap if available
    QDomElement backPathNode = context.selectElement(node, "PathBack");
    if (!backPathNode.isNull()) {
        // The implicit default in <1.12.0 was FIXED so we keep it for backwards
        // compatibility.
        setPixmapBackground(
                context.getPixmapSource(backPathNode),
                context.selectScaleMode(backPathNode, Paintable::FIXED),
                context.getScaleFactor());
    }

    QDomElement vuNode = context.selectElement(node, "PathVu");
    // The implicit default in <1.12.0 was FIXED so we keep it for backwards
    // compatibility.
    setPixmaps(context.getPixmapSource(vuNode), bHorizontal,
               context.selectScaleMode(vuNode, Paintable::FIXED),
               context.getScaleFactor());

    m_iPeakHoldSize = context.selectInt(node, "PeakHoldSize");
    if (m_iPeakHoldSize < 0 || m_iPeakHoldSize > 100) {
        m_iPeakHoldSize = DEFAULT_HOLDSIZE;
    }

    m_iPeakFallStep = context.selectInt(node, "PeakFallStep");
    if (m_iPeakFallStep < 1 || m_iPeakFallStep > 1000) {
        m_iPeakFallStep = DEFAULT_FALLSTEP;
    }

    m_iPeakHoldTime = context.selectInt(node, "PeakHoldTime");
    if (m_iPeakHoldTime < 1 || m_iPeakHoldTime > 3000) {
        m_iPeakHoldTime = DEFAULT_HOLDTIME;
    }

    m_iPeakFallTime = context.selectInt(node, "PeakFallTime");
    if (m_iPeakFallTime < 1 || m_iPeakFallTime > 1000) {
        m_iPeakFallTime = DEFAULT_FALLTIME;
    }

    setFocusPolicy(Qt::NoFocus);
}

void WVuMeter::setPixmapBackground(
        const PixmapSource& source,
        Paintable::DrawMode mode,
        double scaleFactor) {
    m_pPixmapBack = WPixmapStore::getPaintable(source, mode, scaleFactor);
    if (m_pPixmapBack.isNull() || m_pPixmapBack->isNull()) {
        qDebug() << metaObject()->className()
                 << "Error loading background pixmap:" << source.getPath();
    } else if (mode == Paintable::FIXED) {
        setFixedSize(m_pPixmapBack->size());
    }
}

void WVuMeter::setPixmaps(const PixmapSource& source,
        bool bHorizontal,
        Paintable::DrawMode mode,
        double scaleFactor) {
    m_pPixmapVu = WPixmapStore::getPaintable(source, mode, scaleFactor);
    if (m_pPixmapVu.isNull() || m_pPixmapVu->isNull()) {
        qDebug() << "WVuMeter: Error loading vu pixmap" << source.getPath();
    } else {
        m_bHorizontal = bHorizontal;
        if (m_bHorizontal) {
            m_iPixmapLength = m_pPixmapVu->width();
        } else {
            m_iPixmapLength = m_pPixmapVu->height();
        }
    }
}

void WVuMeter::onConnectedControlChanged(double dParameter, double dValue) {
    Q_UNUSED(dValue);
    m_dParameter = math_clamp(dParameter, 0.0, 1.0);

    if (dParameter > 0.0) {
        setPeak(dParameter);
    } else {
        // A 0.0 value is very unlikely except when the VU Meter is disabled
        m_dPeakParameter = 0;
    }

    updateState(m_timer.restart());
}

void WVuMeter::setPeak(double parameter) {
    if (parameter > m_dPeakParameter) {
        m_dPeakParameter = parameter;
        m_dPeakHoldCountdownMs = m_iPeakHoldTime;
    }
}

void WVuMeter::updateState(mixxx::Duration elapsed) {
    double msecsElapsed = elapsed.toDoubleMillis();
    // If we're holding at a peak then don't update anything
    m_dPeakHoldCountdownMs -= msecsElapsed;
    if (m_dPeakHoldCountdownMs > 0) {
        return;
    } else {
        m_dPeakHoldCountdownMs = 0;
    }

    // Otherwise, decrement the peak position by the fall step size times the
    // milliseconds elapsed over the fall time multiplier. The peak will fall
    // FallStep times (out of 128 steps) every FallTime milliseconds.
    m_dPeakParameter -= static_cast<double>(m_iPeakFallStep) *
            msecsElapsed /
            static_cast<double>(m_iPeakFallTime * m_iPixmapLength);
    m_dPeakParameter = math_clamp(m_dPeakParameter, 0.0, 1.0);
}

void WVuMeter::maybeUpdate() {
    if (m_dParameter != m_dLastParameter || m_dPeakParameter != m_dLastPeakParameter) {
        repaint();
    }
}

void WVuMeter::paintEvent(QPaintEvent * /*unused*/) {
    ScopedTimer t("WVuMeter::paintEvent");

    QStyleOption option;
    option.initFrom(this);
    QStylePainter p(this);
    p.drawPrimitive(QStyle::PE_Widget, option);

    if (!m_pPixmapBack.isNull() && !m_pPixmapBack->isNull()) {
        // Draw background. DrawMode takes care of whether to stretch or not.
        m_pPixmapBack->draw(rect(), &p);
    }

    if (!m_pPixmapVu.isNull() && !m_pPixmapVu->isNull()) {
        const double widgetWidth = width();
        const double widgetHeight = height();
        const double pixmapWidth = m_pPixmapVu->width();
        const double pixmapHeight = m_pPixmapVu->height();

        // Draw (part of) vu
        if (m_bHorizontal) {
            const double widgetPosition = math_clamp(widgetWidth * m_dParameter,
                                                     0.0, widgetWidth);
            QRectF targetRect(0, 0, widgetPosition, widgetHeight);

            const double pixmapPosition = math_clamp(pixmapWidth * m_dParameter,
                                                     0.0, pixmapWidth);
            QRectF sourceRect(0, 0, pixmapPosition,  m_pPixmapVu->height());
            m_pPixmapVu->draw(targetRect, &p, sourceRect);

            if (m_iPeakHoldSize > 0 && m_dPeakParameter > 0.0) {
                const double widgetPeakPosition = math_clamp(
                        widgetWidth * m_dPeakParameter, 0.0, widgetWidth);
                const double widgetPeakHoldSize = widgetWidth *
                        static_cast<double>(m_iPeakHoldSize) / pixmapWidth;

                const double pixmapPeakPosition = math_clamp(
                        pixmapWidth * m_dPeakParameter, 0.0, pixmapWidth);
                const double pixmapPeakHoldSize = m_iPeakHoldSize;

                targetRect = QRectF(widgetPeakPosition - widgetPeakHoldSize, 0,
                                    widgetPeakHoldSize, widgetHeight);
                sourceRect = QRectF(pixmapPeakPosition - pixmapPeakHoldSize, 0,
                                    pixmapPeakHoldSize, pixmapHeight);
                m_pPixmapVu->draw(targetRect, &p, sourceRect);
            }
        } else {
            const double widgetPosition = math_clamp(widgetHeight * m_dParameter,
                                                     0.0, widgetHeight);
            QRectF targetRect(0, widgetHeight - widgetPosition,
                              widgetWidth, widgetPosition);

            const double pixmapPosition = math_clamp(pixmapHeight * m_dParameter,
                                                     0.0, pixmapHeight);
            QRectF sourceRect(0, pixmapHeight - pixmapPosition,
                              pixmapWidth, pixmapPosition);
            m_pPixmapVu->draw(targetRect, &p, sourceRect);

            if (m_iPeakHoldSize > 0 && m_dPeakParameter > 0.0) {
                const double widgetPeakPosition = math_clamp(
                        widgetHeight * m_dPeakParameter, 0.0, widgetHeight);
                const double widgetPeakHoldSize = widgetHeight *
                        static_cast<double>(m_iPeakHoldSize) / pixmapHeight;

                const double pixmapPeakPosition = math_clamp(
                        pixmapHeight * m_dPeakParameter, 0.0, pixmapHeight);
                const double pixmapPeakHoldSize = m_iPeakHoldSize;

                targetRect = QRectF(0, widgetHeight - widgetPeakPosition,
                                    widgetWidth, widgetPeakHoldSize);
                sourceRect = QRectF(0, pixmapHeight - pixmapPeakPosition,
                                    pixmapWidth, pixmapPeakHoldSize);
                m_pPixmapVu->draw(targetRect, &p, sourceRect);
            }
        }
    }
    m_dLastParameter = m_dParameter;
    m_dLastPeakParameter = m_dPeakParameter;
}
