#ifndef MIDIOBJECT_H
#define MIDIOBJECT_H

#include "stdio.h"
#include "stdlib.h"
#include "portmidi.h"
#include <vector>
#include "defs.h"
#include <qthread.h>
#include <qobject.h>
//#include <sched.h>
#include "controlpushbutton.h"
#include "controlpotmeter.h"
class ControlPotmeter;
class ControlPushButton;

class MidiObject : public QThread  {
public:
  MidiObject();
  ~MidiObject();
  void addbutton(ControlPushButton* newbutton);
  void removebutton(ControlPushButton* button);
  void addpotmeter(ControlPotmeter* newpotmeter);
  void removepotmeter(ControlPotmeter* potmeter);
 protected:
  PmEvent buffer[2];
  PmStream *midi;
  void run();
  int fd, count, size, no_potmeters, no_buttons;
  std::vector<ControlPushButton*> buttons;
  std::vector<ControlPotmeter*> potmeters;
};

#endif
