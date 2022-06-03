#include "widget/wknobcomposed.h"

#include <QStyleOption>
#include <QStylePainter>
#include <QTransform>

#include "moc_wknobcomposed.cpp"
#include "util/duration.h"
#include "widget/wskincolor.h"

WKnobComposed::WKnobComposed(QWidget* pParent)
        : WWidget(pParent),
          m_dCurrentAngle(140.0),
          m_dMinAngle(-230.0),
          m_dMaxAngle(50.0),
          m_dKnobCenterXOffset(0),
          m_dKnobCenterYOffset(0),
          m_dArcRadius(0),
          m_dArcThickness(0),
          m_dArcBgThickness(0),
          m_arcUnipolar(true),
          m_arcReversed(false),
          m_arcPenCap(Qt::FlatCap),
          m_renderTimer(mixxx::Duration::fromMillis(20),
                        mixxx::Duration::fromSeconds(1)) {
    connect(&m_renderTimer,
            &WidgetRenderTimer::update,
            this,
            QOverload<>::of(&QWidget::update));
}

void WKnobComposed::setup(const QDomNode& node, const SkinContext& context) {
    clear();

    double scaleFactor = context.getScaleFactor();

    // Set background pixmap if available
    QDomElement backPathElement = context.selectElement(node, "BackPath");
    if (!backPathElement.isNull()) {
        setPixmapBackground(
                context.getPixmapSource(backPathElement),
                context.selectScaleMode(backPathElement, Paintable::STRETCH),
                scaleFactor);
    }

    // Set knob pixmap if available
    QDomElement knobNode = context.selectElement(node, "Knob");
    if (!knobNode.isNull()) {
        setPixmapKnob(
                context.getPixmapSource(knobNode),
                context.selectScaleMode(knobNode, Paintable::STRETCH),
                scaleFactor);
    }

    context.hasNodeSelectDouble(node, "MinAngle", &m_dMinAngle);
    context.hasNodeSelectDouble(node, "MaxAngle", &m_dMaxAngle);
    context.hasNodeSelectDouble(node, "KnobCenterXOffset", &m_dKnobCenterXOffset);
    context.hasNodeSelectDouble(node, "KnobCenterYOffset", &m_dKnobCenterYOffset);
    context.hasNodeSelectDouble(node, "ArcRadius", &m_dArcRadius);

    if (m_dArcRadius > 0.0) {
        context.hasNodeSelectDouble(node, "ArcThickness", &m_dArcThickness);
        context.hasNodeSelectDouble(node, "ArcBgThickness", &m_dArcBgThickness);
        if (m_dArcThickness > 0.0) {
            m_dArcThickness *= scaleFactor;
            m_arcColor = WSkinColor::getCorrectColor(context.selectColor(node, "ArcColor"));
        }
        if (m_dArcBgThickness > 0.0) {
            m_dArcBgThickness *= scaleFactor;
            m_arcBgColor = WSkinColor::getCorrectColor(context.selectColor(node, "ArcBgColor"));
        }
        if (context.selectBool(node, "ArcRoundCaps", false)) {
            m_arcPenCap = Qt::RoundCap;
        }
        // ToDo: Make these properties configurable by the connected control.
        // Example: Meta knobs that are fully dry when centered, or parameters
        // that work reversed, like microphone ducking or BitCrusher parameters.
        m_arcUnipolar = context.selectBool(node, "ArcUnipolar", false);
        m_arcReversed = context.selectBool(node, "ArcReversed", false);
    }

    m_dKnobCenterXOffset *= scaleFactor;
    m_dKnobCenterYOffset *= scaleFactor;

    setFocusPolicy(Qt::NoFocus);
}

void WKnobComposed::clear() {
    m_pPixmapBack.clear();
    m_pKnob.clear();
}

void WKnobComposed::setPixmapBackground(const PixmapSource& source,
        Paintable::DrawMode mode,
        double scaleFactor) {
    m_pPixmapBack = WPixmapStore::getPaintable(source, mode, scaleFactor);
    if (m_pPixmapBack.isNull() || m_pPixmapBack->isNull()) {
        qDebug() << metaObject()->className()
                 << "Error loading background pixmap:" << source.getPath();
    }
}

void WKnobComposed::setPixmapKnob(const PixmapSource& source,
        Paintable::DrawMode mode,
        double scaleFactor) {
    m_pKnob = WPixmapStore::getPaintable(source, mode, scaleFactor);
    if (m_pKnob.isNull() || m_pKnob->isNull()) {
        qDebug() << metaObject()->className()
                 << "Error loading knob pixmap:" << source.getPath();
    }
}

void WKnobComposed::onConnectedControlChanged(double dParameter, double dValue) {
    Q_UNUSED(dValue);
    // dParameter is in the range [0, 1].
    double angle = m_dMinAngle + (m_dMaxAngle - m_dMinAngle) * dParameter;

    // TODO(rryan): What's a good epsilon? Should it be dependent on the min/max
    // angle range? Right now it's just 1/100th of a degree.
    if (fabs(angle - m_dCurrentAngle) > 0.01) {
        // paintEvent updates m_dCurrentAngle
        update();
    }
}

void WKnobComposed::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    QStyleOption option;
    option.initFrom(this);
    QStylePainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.drawPrimitive(QStyle::PE_Widget, option);

    if (m_pPixmapBack) {
        m_pPixmapBack->draw(rect(), &p, m_pPixmapBack->rect());
    }

    if ((!m_pKnob.isNull() && !m_pKnob->isNull()) || m_dArcRadius > 0.1) {
        // We update m_dCurrentAngle since onConnectedControlChanged uses it for
        // no-op detection.
        m_dCurrentAngle = m_dMinAngle + (m_dMaxAngle - m_dMinAngle) * getControlParameterDisplay();
    }

    if (m_dArcRadius > 0.1) {
        drawArc(&p);
    }

    QTransform transform;
    if (!m_pKnob.isNull() && !m_pKnob->isNull()) {
        qreal tx = m_dKnobCenterXOffset + width() / 2.0;
        qreal ty = m_dKnobCenterYOffset + height() / 2.0;
        transform.translate(-tx, -ty);
        p.translate(tx, ty);

        p.rotate(m_dCurrentAngle);

        // Need to convert from QRect to a QRectF to avoid losing precision.
        QRectF targetRect = rect();
        m_pKnob->drawCentered(transform.mapRect(targetRect), &p,
                              m_pKnob->rect());
    }
}

void WKnobComposed::drawArc(QPainter* pPainter) {
    // In order to always draw the arcs undistorted we set up
    // a quadratic target rectangle regardless of the widget's
    // width-to-height ratio.
    qreal centerX = width() / 2.0 + m_dKnobCenterXOffset;
    qreal centerY = height() / 2.0 + m_dKnobCenterYOffset;
    QPointF topLeft = QPointF((centerX - m_dArcRadius), (centerY - m_dArcRadius));
    QPointF bottomRight = QPointF((centerX + m_dArcRadius), (centerY + m_dArcRadius));
    QRectF rect = QRectF(topLeft, bottomRight);

    // draw background arc
    if (m_dArcBgThickness > 0.0) {
        QPen arcBgPen = QPen(m_arcBgColor);
        arcBgPen.setWidthF(m_dArcBgThickness);
        arcBgPen.setCapStyle(m_arcPenCap);
        pPainter->setPen(arcBgPen);
        pPainter->drawArc(rect,
                static_cast<int>((90 - m_dMinAngle) * 16),
                static_cast<int>((m_dMinAngle - m_dMaxAngle) * 16));
    }

    // draw foreground arc
    QPen arcPen = QPen(m_arcColor);
    arcPen.setWidthF(m_dArcThickness);
    arcPen.setCapStyle(m_arcPenCap);

    pPainter->setPen(arcPen);
    if (m_arcUnipolar) {
        if (m_arcReversed) {
           // draw arc from maxAngle to current position
           pPainter->drawArc(rect,
                   static_cast<int>((90 - m_dCurrentAngle) * 16),
                   static_cast<int>((m_dMaxAngle - m_dCurrentAngle) * -16));
        } else {
            // draw arc from minAngle to current position
            pPainter->drawArc(rect,
                    static_cast<int>((90 - m_dMinAngle) * 16),
                    static_cast<int>((m_dCurrentAngle - m_dMinAngle) * -16));
        }
    } else {
        // draw arc from center to current position
        pPainter->drawArc(rect, 90 * 16, static_cast<int>(m_dCurrentAngle * -16));
    }
}

void WKnobComposed::mouseMoveEvent(QMouseEvent* e) {
    m_handler.mouseMoveEvent(this, e);
}

void WKnobComposed::mousePressEvent(QMouseEvent* e) {
    m_handler.mousePressEvent(this, e);
}

void WKnobComposed::mouseDoubleClickEvent(QMouseEvent* e) {
    m_handler.mouseDoubleClickEvent(this, e);
}

void WKnobComposed::mouseReleaseEvent(QMouseEvent* e) {
    m_handler.mouseReleaseEvent(this, e);
}

void WKnobComposed::wheelEvent(QWheelEvent* e) {
    m_handler.wheelEvent(this, e);
}

void WKnobComposed::inputActivity() {
#ifdef __APPLE__
    m_renderTimer.activity();
#else
    update();
#endif
}
