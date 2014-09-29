#include <QStylePainter>
#include <QStyleOption>

#include "widget/wknobcomposed.h"

WKnobComposed::WKnobComposed(QWidget* pParent)
        : WWidget(pParent),
          m_dCurrentAngle(140.0),
          m_dMinAngle(-230.0),
          m_dMaxAngle(50.0) {
}

WKnobComposed::~WKnobComposed() {
}

void WKnobComposed::setup(QDomNode node, const SkinContext& context) {
    clear();

    // Set background pixmap if available
    if (context.hasNode(node, "BackPath")) {
        QString mode_str = context.selectAttributeString(
                context.selectElement(node, "BackPath"), "scalemode", "TILE");
        setPixmapBackground(context.getPixmapSource(context.selectNode(node, "BackPath")),
                            Paintable::DrawModeFromString(mode_str));
    }

    // Set background pixmap if available
    if (context.hasNode(node, "Knob")) {
        setPixmapKnob(context.getPixmapSource(context.selectNode(node, "Knob")));
    }

    if (context.hasNode(node, "MinAngle")) {
        m_dMinAngle = context.selectDouble(node, "MinAngle");
    }

    if (context.hasNode(node, "MaxAngle")) {
        m_dMaxAngle = context.selectDouble(node, "MaxAngle");
    }
}

void WKnobComposed::clear() {
    m_pPixmapBack.clear();
    m_pKnob.clear();
}

void WKnobComposed::setPixmapBackground(PixmapSource* pSource,
                                        Paintable::DrawMode mode) {
    m_pPixmapBack = WPixmapStore::getPaintable(pSource, mode);
    if (m_pPixmapBack.isNull() || m_pPixmapBack->isNull()) {
        qDebug() << metaObject()->className()
                 << "Error loading background pixmap:" << pSource->getPath();
    }
}

void WKnobComposed::setPixmapKnob(PixmapSource* pSource) {
    m_pKnob = WPixmapStore::getPaintable(pSource, Paintable::STRETCH);
    if (m_pKnob.isNull() || m_pKnob->isNull()) {
        qDebug() << metaObject()->className()
                 << "Error loading knob pixmap:" << pSource->getPath();
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
        m_pPixmapBack->draw(0, 0, &p);
    }

    if (!m_pKnob.isNull() && !m_pKnob->isNull()) {
        p.translate(width() / 2.0, height() / 2.0);

        // We update m_dCurrentAngle since onConnectedControlChanged uses it for
        // no-op detection.
        m_dCurrentAngle = m_dMinAngle + (m_dMaxAngle - m_dMinAngle) * getControlParameterDisplay();
        p.rotate(m_dCurrentAngle);

        m_pKnob->draw(-m_pKnob->width() / 2.0, -m_pKnob->height() / 2.0, &p);
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
