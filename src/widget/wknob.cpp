#include "widget/wknob.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QtDebug>

#include "moc_wknob.cpp"
#include "util/duration.h"

WKnob::WKnob(QWidget* pParent)
        : WDisplay(pParent) {
    setFocusPolicy(Qt::NoFocus);
}

void WKnob::mouseMoveEvent(QMouseEvent* e) {
    m_handler.mouseMoveEvent(this, e);
}

void WKnob::mousePressEvent(QMouseEvent* e) {
    m_handler.mousePressEvent(this, e);
}

void WKnob::mouseDoubleClickEvent(QMouseEvent* e) {
    m_handler.mouseDoubleClickEvent(this, e);
}

void WKnob::mouseReleaseEvent(QMouseEvent* e) {
    m_handler.mouseReleaseEvent(this, e);
}

void WKnob::wheelEvent(QWheelEvent* e) {
    m_handler.wheelEvent(this, e);
}

void WKnob::inputActivity() {
    update();
}
