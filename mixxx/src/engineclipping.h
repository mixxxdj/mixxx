#ifndef ENGINECLIPPING_H
#define ENGINECLIPPING_H

#include "wbulb.h"
#include "engineobject.h"

class EngineClipping : public EngineObject {
 private:
    WBulb *bulb_clipping;

    CSAMPLE *buffer;
 public:
    EngineClipping(WBulb *);
    ~EngineClipping();
    CSAMPLE *process(const CSAMPLE *, const int);
};

#endif


















