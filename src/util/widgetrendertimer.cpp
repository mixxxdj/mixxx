#include "util/widgetrendertimer.h"

#include "util/time.h"

WidgetRenderTimer::WidgetRenderTimer(mixxx::Duration renderFrequency,
                                     mixxx::Duration inactivityTimeout)
        :  m_renderFrequency(renderFrequency),
           m_inactivityTimeout(inactivityTimeout),
           m_guiTickTimer(this) {
    connect(&m_guiTickTimer, SIGNAL(timeout()),
            this, SLOT(guiTick()));
}

void WidgetRenderTimer::guiTick() {
    mixxx::Duration now = mixxx::Time::elapsed();
    if (now - m_lastActivity > mixxx::Duration::fromSeconds(1)) {
        m_guiTickTimer.stop();
    }
    if (m_lastActivity > m_lastRender) {
        m_lastRender = m_lastActivity;
        emit(update());
    }
}

void WidgetRenderTimer::activity() {
    // Bug #1793015: With Qt 4, we would simply call QWidget::update in response
    // to input events that required re-rendering widgets, relying on Qt to
    // batch them together and deliver them at a reasonable frequency. On macOS,
    // the behavior of QWidget::update in Qt 5 seems to have changed such that
    // render events happen much more frequently than they used to. To address
    // this, we instead use a downsampling timer attached to the VSyncThread's
    // render ticks for the waveform renderers. The timer invokes guiTick(),
    // which is responsible for actually calling QWidget::update(). When input
    // arrives, we call inputActivity to attach the timer. After 1 second of
    // inactivity, we disconnect the timer.
    m_lastActivity = mixxx::Time::elapsed();
    if (!m_guiTickTimer.isActive()) {
        m_guiTickTimer.start(mixxx::Duration::fromMillis(20));
    }
}
