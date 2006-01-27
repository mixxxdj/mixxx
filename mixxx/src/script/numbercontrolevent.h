#ifndef NUMBERCONTROLEVENT_H
#define NUMBERCONTROLEVENT_H

#include "../controlobject.h"
#include "scriptcontrolevent.h"
#include <qdatetime.h>

/**
@author 
*/
class NumberControlEvent : public ScriptControlEvent {
public:
    NumberControlEvent(const char* group, const char* name, double value, QDateTime time);
    NumberControlEvent(ControlObject *obj, double value, QDateTime time);

    virtual ~NumberControlEvent();
    virtual void execute();
    double getValue();
    
protected:
    ControlObject* m_obj;
    double m_value;
};

#endif
