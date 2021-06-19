#include "util/widgetrendertimer.h"

#include "moc_widgetrendertimer.cpp"
#include "util/time.h"

WidgetRenderTimer::WidgetRenderTimer(mixxx::Duration renderFrequency,
                                     mixxx::Duration inactivityTimeout)
        : m_renderFrequency(renderFrequency),
          m_inactivityTimeout(inactivityTimeout),
          m_guiTickTimer(this) {
    connect(&m_guiTickTimer, &GuiTickTimer::timeout, this, &WidgetRenderTimer::guiTick);
}

void WidgetRenderTimer::guiTick() {
    mixxx::Duration now = mixxx::Time::elapsed();
    if (now - m_lastActivity > m_inactivityTimeout) {
        m_guiTickTimer.stop();
    }
    if (m_lastActivity > m_lastRender) {
        m_lastRender = m_lastActivity;
        emit update();
    }
}

void WidgetRenderTimer::activity() {
    m_lastActivity = mixxx::Time::elapsed();
    if (!m_guiTickTimer.isActive()) {
        m_guiTickTimer.start(m_renderFrequency);
    }
}
