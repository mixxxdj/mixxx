#include "controlttrotary.h"


/* -------- ------------------------------------------------------
   Purpose: Creates a new rotary encoder
   Input:   key
   -------- ------------------------------------------------------ */
ControlTTRotary::ControlTTRotary(ConfigKey key) : ControlObject(key)
{
    value = 0.;
    received = 0;
    
    // Connect 10ms timer, and start
    //connect(&timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
    //timer.start(100,false);
}

void ControlTTRotary::slotSetPosition(int v)
{
    value = ((float)v-64.)/200.;
    qDebug("%f",value);
    emit(valueChanged(value));
/*    if (v==0)
        received--;
    else
        received++;
*/
}

FLOAT_TYPE ControlTTRotary::getValue()
{
    return value;
}

void ControlTTRotary::slotSetValue(int newvalue)
{
    value = ((FLOAT_TYPE)newvalue-49.)/100.;
    emit(valueChanged(value));
}

void ControlTTRotary::slotTimer()
{
    FLOAT_TYPE newv = (FLOAT_TYPE)received/1000.;
    received = 0;
    
    if (newv != value)
    {
        value = newv;

        emit(valueChanged(value));
        //updateGUI();
    }
    //qDebug("rotary: %f",value);
}
