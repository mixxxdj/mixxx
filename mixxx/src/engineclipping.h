#ifndef ENGINECLIPPING_H
#define ENGINECLIPPING_H

#include "qradiobutton.h"
#include "engineobject.h"

class EngineClipping : public EngineObject {
 private:
    QRadioButton *bulb_clipping;
 public:
    EngineClipping(QRadioButton *);
    ~EngineClipping();
    CSAMPLE *process(CSAMPLE *, int);
};

#endif


















