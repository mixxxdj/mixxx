#ifndef BATTERY_H
#define BATTERY_H

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
    static Battery* getBattery(QObject* parent=NULL);
    virtual ~Battery();

    // returns time until discharged or time until fully charged
    // dependent on the current state
    int getMinutesLeft() { return m_iMinutesLeft; }
    int getPercentage() { return m_iPercentage; }
    ChargingState getChargingState() { return m_csChargingState; }

  public slots:
    // calls read(), checks if state changed, invokes stateChanged
    virtual void update();

  signals:
    // signal is emitted if the percentage or the charging state changed
    void stateChanged();

  protected:
    Battery(QObject* parent=NULL);
    virtual void read() = 0;

    ChargingState m_csChargingState;
    int m_iPercentage;
    int m_iMinutesLeft;

  private:
    QTimer timer;
};

#endif /* BATTERY_H */
