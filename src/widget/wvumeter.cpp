#include "widget/wvumeter.h"

#include "moc_wvumeter.cpp"
#include "util/duration.h"
#include "util/math.h"

WVuMeter::WVuMeter(QWidget* pParent)
        : WVuMeterBase(pParent) {
}

void WVuMeter::draw() {
    QPainter p(paintDevice());
    // fill the background, in case the image contains transparency
    p.fillRect(rect(), m_qBgColor);

    if (!m_pPixmapBack.isNull()) {
        // Draw background.
        QRectF sourceRect(0, 0, m_pPixmapBack->width(), m_pPixmapBack->height());
        m_pPixmapBack->draw(rect(), &p, sourceRect);
    }

    const double widgetWidth = width();
    const double widgetHeight = height();
    const double pixmapWidth = m_pPixmapVu.isNull() ? 0 : m_pPixmapVu->width();
    const double pixmapHeight = m_pPixmapVu.isNull() ? 0 : m_pPixmapVu->height();

    // Draw (part of) vu
    if (m_bHorizontal) {
        {
            const double widgetPosition = math_clamp(widgetWidth * m_dParameter, 0.0, widgetWidth);
            QRectF targetRect(0, 0, widgetPosition, widgetHeight);

            if (!m_pPixmapVu.isNull()) {
                const double pixmapPosition = math_clamp(
                        pixmapWidth * m_dParameter, 0.0, pixmapWidth);
                QRectF sourceRect(0, 0, pixmapPosition, pixmapHeight);
                m_pPixmapVu->draw(targetRect, &p, sourceRect);
            } else {
                // fallback to green rectangle
                p.fillRect(targetRect, QColor(0, 255, 0));
            }
        }

        if (m_iPeakHoldSize > 0 && m_dPeakParameter > 0.0 &&
                m_dPeakParameter > m_dParameter) {
            const double widgetPeakPosition = math_clamp(
                    widgetWidth * m_dPeakParameter, 0.0, widgetWidth);
            const double pixmapPeakHoldSize = static_cast<double>(m_iPeakHoldSize);
            const double widgetPeakHoldSize = widgetWidth * pixmapPeakHoldSize / pixmapWidth;

            QRectF targetRect(widgetPeakPosition - widgetPeakHoldSize,
                    0,
                    widgetPeakHoldSize,
                    widgetHeight);

            if (!m_pPixmapVu.isNull()) {
                const double pixmapPeakPosition = math_clamp(
                        pixmapWidth * m_dPeakParameter, 0.0, pixmapWidth);

                QRectF sourceRect =
                        QRectF(pixmapPeakPosition - pixmapPeakHoldSize,
                                0,
                                pixmapPeakHoldSize,
                                pixmapHeight);
                m_pPixmapVu->draw(targetRect, &p, sourceRect);
            } else {
                // fallback to green rectangle
                p.fillRect(targetRect, QColor(0, 255, 0));
            }
        }
    } else {
        // vertical
        {
            const double widgetPosition =
                    math_clamp(widgetHeight * m_dParameter, 0.0, widgetHeight);
            QRectF targetRect(0, widgetHeight - widgetPosition, widgetWidth, widgetPosition);

            if (!m_pPixmapVu.isNull()) {
                const double pixmapPosition = math_clamp(
                        pixmapHeight * m_dParameter, 0.0, pixmapHeight);
                QRectF sourceRect(0, pixmapHeight - pixmapPosition, pixmapWidth, pixmapPosition);
                m_pPixmapVu->draw(targetRect, &p, sourceRect);
            } else {
                // fallback to green rectangle
                p.fillRect(targetRect, QColor(0, 255, 0));
            }
        }

        if (m_iPeakHoldSize > 0 && m_dPeakParameter > 0.0 &&
                m_dPeakParameter > m_dParameter) {
            const double widgetPeakPosition = math_clamp(
                    widgetHeight * m_dPeakParameter, 0.0, widgetHeight);
            const double pixmapPeakHoldSize = static_cast<double>(m_iPeakHoldSize);
            const double widgetPeakHoldSize = widgetHeight * pixmapPeakHoldSize / pixmapHeight;

            QRectF targetRect(0,
                    widgetHeight - widgetPeakPosition,
                    widgetWidth,
                    widgetPeakHoldSize);

            if (!m_pPixmapVu.isNull()) {
                const double pixmapPeakPosition = math_clamp(
                        pixmapHeight * m_dPeakParameter, 0.0, pixmapHeight);

                QRectF sourceRect = QRectF(0,
                        pixmapHeight - pixmapPeakPosition,
                        pixmapWidth,
                        pixmapPeakHoldSize);
                m_pPixmapVu->draw(targetRect, &p, sourceRect);
            } else {
                // fallback to green rectangle
                p.fillRect(targetRect, QColor(0, 255, 0));
            }
        }
    }
}
