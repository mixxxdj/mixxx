#include "widget/wvumeterbase.h"

#include "skin/legacy/skincontext.h"
#include "util/math.h"
#include "util/timer.h"
#include "util/widgethelper.h"
#include "waveform/vsyncthread.h"
#include "widget/moc_wvumeterbase.cpp"
#include "widget/wpixmapstore.h"

#define DEFAULT_FALLTIME 20
#define DEFAULT_FALLSTEP 1
#define DEFAULT_HOLDTIME 400
#define DEFAULT_HOLDSIZE 5

WVuMeterBase::WVuMeterBase(QWidget* parent)
        : WGLWidget(parent),
          WBaseWidget(this),
          m_iPendingRenders(0),
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
}

WVuMeterBase::~WVuMeterBase() {
}

void WVuMeterBase::setup(const QDomNode& node, const SkinContext& context) {
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

    if (height() < 2 || width() < 2) {
        // This triggers a QT bug and displays a white widget instead.
        // We warn here, because the skin designer may not use the affected mode.
        SKIN_WARNING(node,
                context,
                QStringLiteral("VuMeterBase needs to have 2 pixel in all "
                               "extents to be visible on all targets."));
    }

    setFocusPolicy(Qt::NoFocus);
}

void WVuMeterBase::setPixmapBackground(
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

void WVuMeterBase::setPixmaps(const PixmapSource& source,
        bool bHorizontal,
        Paintable::DrawMode mode,
        double scaleFactor) {
    m_pPixmapVu = WPixmapStore::getPaintable(source, mode, scaleFactor);
    if (m_pPixmapVu.isNull()) {
        qDebug() << "WVuMeterBase: Error loading vu pixmap" << source.getPath();
    } else {
        m_bHorizontal = bHorizontal;
        if (m_bHorizontal) {
            m_iPixmapLength = m_pPixmapVu->width();
        } else {
            m_iPixmapLength = m_pPixmapVu->height();
        }
    }
}

void WVuMeterBase::onConnectedControlChanged(double dParameter, double dValue) {
    Q_UNUSED(dValue);
    m_dParameter = math_clamp(dParameter, 0.0, 1.0);

    if (dParameter > 0.0) {
        setPeak(dParameter);
    } else {
        // A 0.0 value is very unlikely except when the VU Meter is disabled
        m_dPeakParameter = 0;
    }
}

void WVuMeterBase::setPeak(double parameter) {
    if (parameter > m_dPeakParameter) {
        m_dPeakParameter = parameter;
        m_dPeakHoldCountdownMs = m_iPeakHoldTime;
    }
}

void WVuMeterBase::updateState(mixxx::Duration elapsed) {
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

void WVuMeterBase::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    // Force rerendering when render is called from the vsync thread, e.g. to
    // git rid artifacts after hiding and showing the mixer or incomplete
    // initial drawing. Use 2 passes, in case triple buffering is used.
    m_iPendingRenders = 2;
}

void WVuMeterBase::showEvent(QShowEvent* e) {
    Q_UNUSED(e);
    WGLWidget::showEvent(e);
    // Find the base color recursively in parent widget.
    m_qBgColor = mixxx::widgethelper::findBaseColor(this);
    // Force a rerender when exposed (needed when using QOpenGL)
    // 2 pendings renders, in case we have triple buffering
    m_iPendingRenders = 2;
}

void WVuMeterBase::render(VSyncThread* vSyncThread) {
    if (!shouldRender()) {
        return;
    }

    ScopedTimer t(u"WVuMeterBase::render");

    updateState(vSyncThread->sinceLastSwap());

    if (m_dParameter != m_dLastParameter || m_dPeakParameter != m_dLastPeakParameter) {
        m_iPendingRenders = 2;
    }

    if (m_iPendingRenders == 0) {
        return;
    }

    draw();

    m_dLastParameter = m_dParameter;
    m_dLastPeakParameter = m_dPeakParameter;
    m_iPendingRenders--;
    m_bSwapNeeded = true;
}

void WVuMeterBase::swap() {
    if (!m_bSwapNeeded || !shouldRender()) {
        return;
    }
    makeCurrentIfNeeded();
    swapBuffers();
    doneCurrent();
    m_bSwapNeeded = false;
}
