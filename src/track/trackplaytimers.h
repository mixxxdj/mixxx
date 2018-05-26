#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QTimer>

#include "control/controlproxy.h"

namespace TrackTimers {
class ElapsedTimer {
  public:
    ElapsedTimer() = default;
    virtual ~ElapsedTimer() = default;
    virtual void invalidate() = 0;
    virtual bool isValid() = 0;
    virtual void start() = 0;
    virtual qint64 elapsed() = 0;
};

class RegularTimer : public QObject {
    Q_OBJECT
  public:
    RegularTimer() = default;
    virtual ~RegularTimer() = default;
    virtual void start(int msec) = 0;
    virtual bool isActive() = 0;
  public slots:
    virtual void stop() = 0;
  signals:
    void timeout();
};

class GUITickTimer : public RegularTimer {
    Q_OBJECT
  public:
    GUITickTimer();
    ~GUITickTimer() override = default;
    void start(int msec) override;
    bool isActive() override;
    void stop() override;

  private:
    ControlProxy m_CPGuiTimer50ms;
    double m_msSoFar;
    double m_msTarget;
    bool m_isActive;
    bool m_timeoutSent;
  private slots:
    void slotTick(double timeSinceLastTick);
};

class ElapsedTimerQt : public ElapsedTimer {
  public:
    ElapsedTimerQt() = default;
    ~ElapsedTimerQt() override = default;
    void invalidate() override;
    bool isValid() override;
    void start() override;
    qint64 elapsed() override;

  private:
    QElapsedTimer m_elapsedTimer;
};
} // namespace TrackTimers
