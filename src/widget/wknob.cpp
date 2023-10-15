#include "widget/wknob.h"

#include "moc_wknob.cpp"

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

void WKnob::leaveEvent(QEvent* e) {
    m_handler.leaveEvent(this, e);
}

void WKnob::inputActivity() {
    update();
}
