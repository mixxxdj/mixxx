#ifndef CONTROLROTARY_H
#define CONTROLROTARY_H

#include "configobject.h"
#include "controlpotmeter.h"
#include "defs.h"
#include <sys/timeb.h>
#include <algorithm>

class ControlRotary : public ControlPotmeter
{
  Q_OBJECT
 private:
  timeb oldtime;
  FLOAT_TYPE counter;
  static const char graycodetable[256];
 public:
  short direction;
  ControlRotary(ConfigObject::ConfigKey *key);
  void updatecounter(int, int SRATE);
  short sign(short);
 public slots:
  void slotSetPosition(int);
  void slotSetPositionMidi(int);

};

#endif
