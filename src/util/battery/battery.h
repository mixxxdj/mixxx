#ifndef UTIL_BATTERY_BATTERY_H
#define UTIL_BATTERY_BATTERY_H

#include "util/timer.h"

class Battery : public QObject {
    Q_OBJECT
  public:
    static constexpr int TIME_UNKNOWN = -1;
    enum ChargingState {
        UNKNOWN,
        DISCHARGING,
        CHARGING,
        CHARGED,
    };
    static Battery* getBattery(QObject* parent=nullptr);
    virtual ~Battery();

    // The number of minutes the battery has remaining to depletion (when
    // m_chargingState is DISCHARGING) or to being fully charged (when
    // m_chargingState is CHARGING).
    int getMinutesLeft() { return m_iMinutesLeft; }

    // The current battery percentage charged (from 0 to 100).
    double getPercentage() { return m_dPercentage; }

    // The charging state of the battery.
    ChargingState getChargingState() { return m_chargingState; }

  public slots:
    // calls read(), checks if state changed, invokes stateChanged
    virtual void update();

  signals:
    // signal is emitted if the percentage or the charging state changed
    void stateChanged();

  protected:
    Battery(QObject* parent=nullptr);
    virtual void read() = 0;

    ChargingState m_chargingState;
    double m_dPercentage;
    int m_iMinutesLeft;

  private:
    GuiTickTimer m_timer;
};

#endif /* UTIL_BATTERY_BATTERY_H */
