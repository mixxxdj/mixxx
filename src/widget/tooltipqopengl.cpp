#include "widget/tooltipqopengl.h"

#include <widget/wglwidget.h>

#include <QStyle>
#include <QTimer>
#include <QToolTip>
#include <memory>

ToolTipQOpenGL::ToolTipQOpenGL() {
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &ToolTipQOpenGL::onTimeout);
}

void ToolTipQOpenGL::onTimeout() {
    if (m_widget) {
        QToolTip::showText(m_pos, m_widget->eventReceiver()->toolTip(), m_widget);
    }
}

ToolTipQOpenGL& ToolTipQOpenGL::singleton() {
    static ToolTipQOpenGL instance;
    return instance;
}

void ToolTipQOpenGL::setActive(bool active) {
    m_active = active;
    if (!m_active) {
        m_timer.stop();
    }
}

void ToolTipQOpenGL::start(WGLWidget* widget, QPoint pos) {
    if (m_active) {
        m_widget = widget;
        m_pos = pos;
        m_timer.start(widget->style()->styleHint(QStyle::SH_ToolTip_WakeUpDelay));
    }
}

void ToolTipQOpenGL::stop(WGLWidget* widget) {
    m_timer.stop();
}
