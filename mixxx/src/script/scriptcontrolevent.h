#ifndef SCRIPTCONTROLEVENT_H
#define SCRIPTCONTROLEVENT_H

#include "../controlobject.h"
#include "scriptcontrolevent.h"
#include <qdatetime.h>

/**
@author 
*/
class ScriptControlEvent{
public:
    ScriptControlEvent(const char* group, const char* name, double value, const QDateTime *time);
    ScriptControlEvent(ControlObject *obj, double value, const QDateTime *time);

    ~ScriptControlEvent();
    void execute();
    const QDateTime* getTime();
    double getValue();
    
protected:
    ControlObject* m_obj;
    double m_value;
    const QDateTime* m_time;

};

#endif
