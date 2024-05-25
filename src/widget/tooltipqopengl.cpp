#include "widget/tooltipqopengl.h"

#include <QStyle>
#include <QToolTip>

#include "moc_tooltipqopengl.cpp"
#include "widget/wglwidget.h"

ToolTipQOpenGL::ToolTipQOpenGL()
        : m_active(true),
          m_pWidget(nullptr) {
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &ToolTipQOpenGL::onTimeout);
}

void ToolTipQOpenGL::onTimeout() {
    if (m_pWidget) {
        QToolTip::showText(m_pos, m_pWidget->toolTip(), m_pWidget);
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

void ToolTipQOpenGL::start(WGLWidget* pWidget, QPoint pos) {
    if (m_active) {
        m_pWidget = pWidget;
        m_pos = pos;
        m_timer.start(pWidget->style()->styleHint(QStyle::SH_ToolTip_WakeUpDelay));
    }
}

void ToolTipQOpenGL::stop() {
    m_timer.stop();
    m_pWidget = nullptr;
}
