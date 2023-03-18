#include "widget/tooltipqopengl.h"

#include <widget/wglwidget.h>

#include <QStyle>
#include <QTimer>
#include <QToolTip>

ToolTipQOpenGL::ToolTipQOpenGL()
        : m_timer(new QTimer()) {
    m_timer->setSingleShot(true);
    connect(m_timer.get(), &QTimer::timeout, this, &ToolTipQOpenGL::onTimeout);
}

void ToolTipQOpenGL::onTimeout() {
    if (m_widget) {
        QToolTip::showText(m_pos, m_widget->toolTip(), m_widget);
    }
}

ToolTipQOpenGL::~ToolTipQOpenGL() {
}

ToolTipQOpenGL& ToolTipQOpenGL::singleton() {
    static ToolTipQOpenGL instance;
    return instance;
}

void ToolTipQOpenGL::setActive(bool active) {
    m_active = active;
    if (!m_active) {
        m_timer->stop();
    }
}

void ToolTipQOpenGL::start(WGLWidget* widget, QPoint pos) {
    if (m_active) {
        m_widget = widget;
        m_pos = pos;
        m_timer->start(widget->style()->styleHint(QStyle::SH_ToolTip_WakeUpDelay));
    }
}

void ToolTipQOpenGL::stop(WGLWidget* widget) {
    m_timer->stop();
}
