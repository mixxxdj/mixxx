#include <QStylePainter>
#include <QStyleOption>
#include <QTransform>

#include "widget/wknobcomposed.h"
#include "widget/controlwidgetconnection.h"

WKnobComposed::WKnobComposed(QWidget* pParent)
        : WWidget(pParent),
          m_dCurrentAngle(140.0),
          m_dNeutralParameter(0.0),
          m_iMaskXOffset(0),
          m_iMaskYOffset(0),
          m_dMinAngle(-230.0),
          m_dMaxAngle(50.0),
          m_dKnobCenterXOffset(0),
          m_dKnobCenterYOffset(0) {
}

WKnobComposed::~WKnobComposed() {
}

void WKnobComposed::setup(QDomNode node, const SkinContext& context) {
    clear();

    // Set background pixmap if available
    if (context.hasNode(node, "BackPath")) {
        QDomElement backPathElement = context.selectElement(node, "BackPath");
        setPixmapBackground(context.getPixmapSource(backPathElement),
                            context.selectScaleMode(backPathElement, Paintable::STRETCH));
    }

    // Set knob pixmap if available
    if (context.hasNode(node, "Knob")) {
        QDomElement knobNode = context.selectElement(node, "Knob");
        setPixmapKnob(context.getPixmapSource(knobNode),
                      context.selectScaleMode(knobNode, Paintable::STRETCH));
    }

    // Set ring pixmap if available
    if (context.hasNode(node, "Ring")) {
        QDomElement ringNode = context.selectElement(node, "Ring");
        setPixmapRing(context.getPixmapSource(ringNode),
                      context.selectScaleMode(ringNode, Paintable::STRETCH));
    }

    if (context.hasNode(node, "RingCenterXOffset")) {
        m_iMaskXOffset = context.selectDouble(node, "MaskXOffset");
    }

    if (context.hasNode(node, "RingCenterYOffset")) {
        m_iMaskYOffset = context.selectDouble(node, "MaskYOffset");
    }

    if (context.hasNode(node, "MinAngle")) {
        m_dMinAngle = context.selectDouble(node, "MinAngle");
    }

    if (context.hasNode(node, "MaxAngle")) {
        m_dMaxAngle = context.selectDouble(node, "MaxAngle");
    }

    if (context.hasNode(node, "KnobCenterXOffset")) {
        m_dKnobCenterXOffset = context.selectDouble(node, "KnobCenterXOffset");
    }

    if (context.hasNode(node, "KnobCenterYOffset")) {
        m_dKnobCenterYOffset = context.selectDouble(node, "KnobCenterYOffset");
    }
}

void WKnobComposed::clear() {
    m_pPixmapBack.clear();
    m_pKnob.clear();
}

void WKnobComposed::setPixmapBackground(PixmapSource source,
                                        Paintable::DrawMode mode) {
    m_pPixmapBack = WPixmapStore::getPaintable(source, mode);
    if (m_pPixmapBack.isNull() || m_pPixmapBack->isNull()) {
        qDebug() << metaObject()->className()
                 << "Error loading background pixmap:" << source.getPath();
    }
}

void WKnobComposed::setPixmapKnob(PixmapSource source,
                                  Paintable::DrawMode mode) {
    m_pKnob = WPixmapStore::getPaintable(source, mode);
    if (m_pKnob.isNull() || m_pKnob->isNull()) {
        qDebug() << metaObject()->className()
                 << "Error loading knob pixmap:" << source.getPath();
    }
}

void WKnobComposed::setPixmapRing(PixmapSource source,
                                  Paintable::DrawMode mode) {
    m_pRing = WPixmapStore::getPaintable(source, mode);
    if (m_pRing.isNull() || m_pRing->isNull()) {
        qDebug() << metaObject()->className()
                 << "Error loading ring pixmap:" << source.getPath();
    }
}

void WKnobComposed::addConnection(ControlParameterWidgetConnection* pConnection) {
    m_connections.append(pConnection);
    if (m_connections.size() == 1) {
        ControlParameterWidgetConnection* defaultConnection = m_connections.at(0);
        if (defaultConnection) {
            m_dNeutralParameter = defaultConnection->neutralParameter();
        }
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

    // We update m_dCurrentAngle since onConnectedControlChanged uses it for
    // no-op detection.
    m_dCurrentAngle = m_dMinAngle + (m_dMaxAngle - m_dMinAngle) * getControlParameterDisplay();

    // Qt measures angles in degrees from 3 o'clock counterclockwise.
    // In Mixxx we measure angles also in degrees but from 12 o'clock clockwise.
    // So: QtAngle = 90 - MixxxAngle
    if (m_pRing) {
        QPainterPath path;
        int w = width();
        int h = height();
        path.moveTo(w/2.0 + m_iMaskXOffset, h/2.0 + m_iMaskYOffset);
        double d = sqrt(pow(w+abs(m_iMaskXOffset),2) + pow(h+abs(m_iMaskYOffset),2));
        double neutralAngle = m_dMinAngle + (m_dMaxAngle - m_dMinAngle) * m_dNeutralParameter;
        path.arcTo(QRectF((w-d)/2.0,(h-d)/2.0,d,d), 90 - neutralAngle, neutralAngle - m_dCurrentAngle);

        path.closeSubpath();
        p.save();
        p.setClipPath(path);
        m_pRing->draw(rect(), &p, m_pRing->rect());
        p.restore();
    }

    QTransform transform;
    if (!m_pKnob.isNull() && !m_pKnob->isNull()) {
        qreal tx = m_dKnobCenterXOffset + width() / 2.0;
        qreal ty = m_dKnobCenterYOffset + height() / 2.0;
        transform.translate(-tx, -ty);
        p.translate(tx, ty);

        p.rotate(m_dCurrentAngle);

        // Need to convert from QRect to a QRectF to avoid losing precison.
        QRectF targetRect = rect();
        m_pKnob->drawCentered(transform.mapRect(targetRect), &p,
                              m_pKnob->rect());
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
