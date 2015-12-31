#ifndef UTIL_BATTERY_BATTERY_H
#define UTIL_BATTERY_BATTERY_H

#include <QTimer>

class Battery : public QObject {
    Q_OBJECT
  public:
    enum ChargingState {
        UNKNOWN,
        DISCHARGING,
        CHARGING,
        CHARGED,
    };
    static Battery* getBattery(QObject* parent=nullptr);
    virtual ~Battery();

    // returns time until discharged or time until fully charged
    // dependent on the current state
    int getMinutesLeft() { return m_iMinutesLeft; }
    double getPercentage() { return m_dPercentage; }
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
    QTimer timer;
};

#endif /* UTIL_BATTERY_BATTERY_H */
