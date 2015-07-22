#include <QStylePainter>
#include <QStyleOption>

#include "widget/wknobcomposed.h"

WKnobComposed::WKnobComposed(QWidget* pParent)
        : WWidget(pParent),
          m_dMinAngle(-230.0),
          m_dMaxAngle(50.0) {
}

WKnobComposed::~WKnobComposed() {
}

void WKnobComposed::setup(QDomNode node) {
    clear();

    // Set background pixmap if available
    if (!selectNode(node, "BackPath").isNull()) {
        setPixmapBackground(getPath(selectNodeQString(node, "BackPath")));
    }

    // Set background pixmap if available
    if (!selectNode(node, "Knob").isNull()) {
        setPixmapKnob(getPath(selectNodeQString(node, "Knob")));
    }

    if (!selectNode(node, "MinAngle").isNull()) {
        m_dMinAngle = selectNodeDouble(node, "MinAngle");
    }

    if (!selectNode(node, "MaxAngle").isNull()) {
        m_dMaxAngle = selectNodeDouble(node, "MaxAngle");
    }
}

void WKnobComposed::clear() {
    m_pPixmapBack.clear();
    m_pKnob.clear();
}

void WKnobComposed::setPixmapBackground(const QString& filename) {
    m_pPixmapBack = WPixmapStore::getPaintable(filename);
    if (m_pPixmapBack.isNull() || m_pPixmapBack->isNull()) {
        qDebug() << metaObject()->className()
                 << "Error loading background pixmap:" << filename;
    }
}

void WKnobComposed::setPixmapKnob(const QString& filename) {
    m_pKnob = WPixmapStore::getPaintable(filename);
    if (m_pKnob.isNull() || m_pKnob->isNull()) {
        qDebug() << metaObject()->className()
                 << "Error loading knob pixmap:" << filename;
    }
}

void WKnobComposed::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    QStyleOption option;
    option.initFrom(this);
    QStylePainter p(this);
    p.drawPrimitive(QStyle::PE_Widget, option);

    if (m_pPixmapBack) {
        m_pPixmapBack->draw(0, 0, &p);
    }

    if (!m_pKnob.isNull() && !m_pKnob->isNull()) {
        p.translate(width() / 2.0, height() / 2.0);

        // Value is now in the range [0, 1].
        double value = getValue() / 127.0;

        double angle = m_dMinAngle + (m_dMaxAngle - m_dMinAngle) * value;
        p.rotate(angle);

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
