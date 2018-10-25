#pragma once

#include <QObject>

#include "util/duration.h"
#include "util/timer.h"

class WidgetRenderTimer : public QObject {
    Q_OBJECT
  public:
    WidgetRenderTimer(mixxx::Duration renderFrequency,
                      mixxx::Duration inactivityTimeout);
    void activity();

  signals:
    void update();

  private slots:
    void guiTick();

  private:
    const mixxx::Duration m_renderFrequency;
    const mixxx::Duration m_inactivityTimeout;
    GuiTickTimer m_guiTickTimer;
    mixxx::Duration m_lastActivity;
    mixxx::Duration m_lastRender;
};
