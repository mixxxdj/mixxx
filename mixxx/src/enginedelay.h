#ifndef ENGINEDELAY_H
#define ENGINEDELAY_H

#include <qobject.h>

#include "engineobject.h"
#include "controlpotmeter.h"
#include "wknob.h"

const int max_delay = 20000; 

class EngineDelay : public EngineObject {
  Q_OBJECT
public:
  EngineDelay(WKnob *);
  ~EngineDelay();
  CSAMPLE *process(const CSAMPLE *, const int);
  ControlPotmeter *potmeter;

public slots:
  void slotUpdate(FLOAT_TYPE);

private:
  CSAMPLE *process_buffer, *delay_buffer;
  int  delay;
  int delay_pos;

};

#endif
