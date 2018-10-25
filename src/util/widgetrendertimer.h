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
