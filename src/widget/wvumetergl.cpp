#include "widget/wvumetergl.h"

#include "moc_wvumetergl.cpp"
#include "util/math.h"
#include "util/timer.h"
#include "util/widgethelper.h"
#include "waveform/sharedglcontext.h"
#include "waveform/vsyncthread.h"
#include "widget/wpixmapstore.h"

#define DEFAULT_FALLTIME 20
#define DEFAULT_FALLSTEP 1
#define DEFAULT_HOLDTIME 400
#define DEFAULT_HOLDSIZE 5

WVuMeterGL::WVuMeterGL(QWidget* parent)
        : QGLWidget(parent, SharedGLContext::getWidget()),
          WBaseWidget(this),
          m_bHasRendered(false),
          m_bSwapNeeded(false),
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
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);

    setAutoFillBackground(false);
    setAutoBufferSwap(false);

    // Not interested in repaint or update calls, as we draw from the vsync thread
    setUpdatesEnabled(false);
}

void WVuMeterGL::setup(const QDomNode& node, const SkinContext& context) {
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
    setPixmaps(context.getPixmapSource(vuNode),
            bHorizontal,
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

void WVuMeterGL::setPixmapBackground(
        const PixmapSource& source,
        Paintable::DrawMode mode,
        double scaleFactor) {
    m_pPixmapBack = WPixmapStore::getPaintable(source, mode, scaleFactor);
    if (m_pPixmapBack.isNull()) {
        qDebug() << metaObject()->className()
                 << "Error loading background pixmap:" << source.getPath();
    } else if (mode == Paintable::FIXED) {
        setFixedSize(m_pPixmapBack->size());
    }
}

void WVuMeterGL::setPixmaps(const PixmapSource& source,
        bool bHorizontal,
        Paintable::DrawMode mode,
        double scaleFactor) {
    m_pPixmapVu = WPixmapStore::getPaintable(source, mode, scaleFactor);
    if (m_pPixmapVu.isNull()) {
        qDebug() << "WVuMeterGL: Error loading vu pixmap" << source.getPath();
    } else {
        m_bHorizontal = bHorizontal;
        if (m_bHorizontal) {
            m_iPixmapLength = m_pPixmapVu->width();
        } else {
            m_iPixmapLength = m_pPixmapVu->height();
        }
    }
}

void WVuMeterGL::onConnectedControlChanged(double dParameter, double dValue) {
    Q_UNUSED(dValue);
    m_dParameter = math_clamp(dParameter, 0.0, 1.0);

    if (dParameter > 0.0) {
        setPeak(dParameter);
    } else {
        // A 0.0 value is very unlikely except when the VU Meter is disabled
        m_dPeakParameter = 0;
    }
}

void WVuMeterGL::setPeak(double parameter) {
    if (parameter > m_dPeakParameter) {
        m_dPeakParameter = parameter;
        m_dPeakHoldCountdownMs = m_iPeakHoldTime;
    }
}

void WVuMeterGL::updateState(mixxx::Duration elapsed) {
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

void WVuMeterGL::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    // Force a rerender when render is called from the vsync thread, e.g. to
    // git rid artifacts after hiding and showing the mixer or incomplete
    // initial drawing.
    m_bHasRendered = false;
}

void WVuMeterGL::showEvent(QShowEvent* e) {
    Q_UNUSED(e);
    // Find the base color recursively in parent widget.
    m_qBgColor = mixxx::widgethelper::findBaseColor(this);
}

void WVuMeterGL::render(VSyncThread* vSyncThread) {
    ScopedTimer t("WVuMeterGL::render");

    updateState(vSyncThread->sinceLastSwap());

    if (m_bHasRendered && m_dParameter == m_dLastParameter &&
            m_dPeakParameter == m_dLastPeakParameter) {
        return;
    }

    if (!isValid() || !isVisible()) {
        return;
    }

    auto* window = windowHandle();
    if (window == nullptr || !window->isExposed()) {
        return;
    }

    QPainter p(this);
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
            const double widgetPeakHoldSize = widgetWidth * pixmapPeakHoldSize / pixmapHeight;

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

    m_dLastParameter = m_dParameter;
    m_dLastPeakParameter = m_dPeakParameter;
    m_bHasRendered = true;
    m_bSwapNeeded = true;
}

void WVuMeterGL::swap() {
    if (!isValid() || !isVisible() || !m_bSwapNeeded) {
        return;
    }
    auto* window = windowHandle();
    if (window == nullptr || !window->isExposed()) {
        return;
    }
    if (context() != QGLContext::currentContext()) {
        makeCurrent();
    }
    swapBuffers();
    m_bSwapNeeded = false;
}
