#ifndef CONTROLROTARY_H
#define CONTROLROTARY_H

#include "configobject.h"
#include "controlpotmeter.h"
#include "defs.h"
#include "controlpushbutton.h"
// #include <sys/time.h>
#include <algorithm>

class ControlRotary : public ControlPotmeter
{
    Q_OBJECT
public:
    ControlRotary(ConfigKey key, ControlPushButton *playbutton);
    void updatecounter(int, int SRATE);
    short direction;
public slots:
    void slotSetPosition(int);
    void slotSetPositionMidi(int);
    void slotSetValue(int newvalue);
private:
    short sign(short);
    //timeval oldtime;
    FLOAT_TYPE counter;
//    static const char graycodetable[256];
    ControlPushButton *play;
};

#endif
