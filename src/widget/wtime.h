// wtime.h
// WTime is a widget showing the current time
// In skin.xml, it is represented by a <Time> node.

#ifndef WTIME_H
#define WTIME_H

#include <QTimer>

#include "widget/wlabel.h"

class WTime: public WLabel {
    Q_OBJECT
  public:
    WTime(QWidget *parent=0);
    virtual ~WTime();
    void setup(QDomNode node);

  private slots:
    void refreshTime();

  private:
    void setTimeFormat(QDomNode node);

    QTimer* m_pTimer;
    QString m_sTimeFormat;
    // m_iInterval defines how often the time will be updated
    short m_iInterval;
    // m_iInterval is set to s_iSecondInterval if seconds are shown
    // otherwise, m_iInterval = s_iMinuteInterval
    static const short s_iSecondInterval = 100;
    static const short s_iMinuteInterval = 1000;
};

#endif /* WTIME_H */
