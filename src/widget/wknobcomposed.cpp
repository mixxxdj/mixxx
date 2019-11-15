#include <QStylePainter>
#include <QStyleOption>
#include <QTransform>

#include "util/duration.h"
#include "widget/wknobcomposed.h"
#include "widget/wskincolor.h"

WKnobComposed::WKnobComposed(QWidget* pParent)
        : WWidget(pParent),
          m_dCurrentAngle(140.0),
          m_dMinAngle(-230.0),
          m_dMaxAngle(50.0),
          m_dKnobCenterXOffset(0),
          m_dKnobCenterYOffset(0),
          m_renderTimer(mixxx::Duration::fromMillis(20),
                        mixxx::Duration::fromSeconds(1)) {
    connect(&m_renderTimer, SIGNAL(update()),
            this, SLOT(update()));
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
    m_dArcThickness = context.selectDouble(node, "ArcThickness");

    if (m_dArcThickness > 0.0) {
        m_arcColor = WSkinColor::getCorrectColor(context.selectColor(node, "ArcColor"));
        m_arcUnipolar = context.selectBool(node, "ArcUnipolar", false);
    }

    m_dKnobCenterXOffset *= scaleFactor;
    m_dKnobCenterYOffset *= scaleFactor;
    m_dArcThickness *= scaleFactor;
}

void WKnobComposed::clear() {
    m_pPixmapBack.clear();
    m_pKnob.clear();
}

void WKnobComposed::setPixmapBackground(PixmapSource source,
                                        Paintable::DrawMode mode,
                                        double scaleFactor) {
    m_pPixmapBack = WPixmapStore::getPaintable(source, mode, scaleFactor);
    if (m_pPixmapBack.isNull() || m_pPixmapBack->isNull()) {
        qDebug() << metaObject()->className()
                 << "Error loading background pixmap:" << source.getPath();
    }
}

void WKnobComposed::setPixmapKnob(PixmapSource source,
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

    if ((!m_pKnob.isNull() && !m_pKnob->isNull()) || m_dArcThickness > 0.0) {
        // We update m_dCurrentAngle since onConnectedControlChanged uses it for
        // no-op detection.
        m_dCurrentAngle = m_dMinAngle + (m_dMaxAngle - m_dMinAngle) * getControlParameterDisplay();
    }

    if (m_dArcThickness > 0.0) {
        drawArc(rect(), &p);
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

void WKnobComposed::drawArc(const QRectF& targetRect, QPainter* pPainter) {
    QMargins margins = QMargins();
    margins += m_dArcThickness / 2.0;
    QRectF rect = targetRect.marginsRemoved(margins);
    QPen pen = QPen(m_arcColor);
    pen.setWidth(m_dArcThickness);
    pen.setCapStyle(Qt::FlatCap);
    pPainter->setPen(pen);
    if (m_arcUnipolar) {
        pPainter->drawArc(rect, 90 * 16 - m_dMinAngle * 16, (m_dCurrentAngle - m_dMinAngle) * -16);
    } else {
        pPainter->drawArc(rect, 90 * 16, m_dCurrentAngle * -16);
    }
}

void WKnobComposed::mouseMoveEvent(QMouseEvent* e) {
    m_handler.mouseMoveEvent(this, e);
}

void WKnobComposed::mousePressEvent(QMouseEvent* e) {
    m_handler.mousePressEvent(this, e);
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
