#ifndef CONTROLTTROTARY_H
#define CONTROLTTROTARY_H

#include "configobject.h"
#include "controlobject.h"
#include "defs.h"
#include <qtimer.h>

/** Turn Table rotary controller class. The turntable rotary sends midi events: 0 when turning
  * backwards, and 1 when turning forward. This class keeps track of it's speed, using a timer
  * interrupt */
class ControlTTRotary : public ControlObject
{
    Q_OBJECT
public:
    ControlTTRotary(ConfigKey key);
    FLOAT_TYPE getValue();
public slots:
    void slotSetPosition(int);
    void slotSetValue(int newvalue);
private:
    /** Decreases/increases received since last timer event */
    int received;    
    FLOAT_TYPE value;
    QTimer timer;
private slots:
    void slotTimer();
};

#endif
