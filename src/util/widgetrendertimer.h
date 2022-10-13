#pragma once

#include <QObject>

#include "util/duration.h"
#include "util/timer.h"

// A helper class for rendering widgets on a timer when they need updating.
// Prevents calling QWidget::update too quickly. Controlled by two parameters:
// - renderFrequency: The frequency to render the widget at in response
//   to updates.
// - inactivityTimeout: The timeout after which the widget's render timer is
//   deactivated.
//
// This class was created in response to issue #9437. With Qt 4, we
// would simply call QWidget::update in response to input events that required
// re-rendering widgets, relying on Qt to batch them together and deliver them
// at a reasonable frequency. On macOS, the behavior of QWidget::update in Qt 5
// seems to have changed such that render events happen much more frequently
// than they used to. To address this, we instead use a downsampling timer
// attached to the VSyncThread's render ticks for the waveform renderers. The
// timer invokes guiTick(), which is responsible for actually calling
// QWidget::update(). When input arrives, we call inputActivity to attach the
// timer. After 1 second of inactivity, we disconnect the timer.
//
// Ironically, using this class somehow causes severe lagginess on mouse input
// with Windows, so use #ifdefs to only call activity() on macOS; just call
// QWidget::update() for other operating systems.
//
// Also when using the QOpenGLWindow based WGLWidget (when MIXXX_USE_QOPENGL is
// defined) using this seems not necessary and makes causes lagginess
#ifdef __APPLE__
#ifndef MIXXX_USE_QOPENGL
#define USE_WIDGET_RENDER_TIMER
#endif
#endif

#ifdef USE_WIDGET_RENDER_TIMER
class WidgetRenderTimer : public QObject {
    Q_OBJECT
  public:
    WidgetRenderTimer(mixxx::Duration renderFrequency,
                      mixxx::Duration inactivityTimeout);

    // Call this method whenever the widget's state has changed such that a
    // re-render is necessary.
    void activity();

  signals:
    // Emitted when the widget should actually render. Connect this signal to
    // QWidget::update or QWidget::repaint.
    void update();

  private slots:
    // Called when the internal GuiTickTimer's timeout is elapsed. Decides
    // whether the widget should render in response to this timer tick.
    void guiTick();

  private:
    const mixxx::Duration m_renderFrequency;
    const mixxx::Duration m_inactivityTimeout;
    GuiTickTimer m_guiTickTimer;
    mixxx::Duration m_lastActivity;
    mixxx::Duration m_lastRender;
};
#endif
