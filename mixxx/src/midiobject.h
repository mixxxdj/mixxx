#ifndef MIDIOBJECT_H
#define MIDIOBJECT_H

#include <sys/asoundlib.h>
#include <stdlib.h>
#include <vector>
#include "defs.h"
#include <qthread.h>
#include <qobject.h>
#include <sched.h>
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
  void run();
  snd_rawmidi_t *handle;
  int fd, count, size, no_potmeters, no_buttons;
  char *buffer;
  vector<ControlPushButton*> buttons;
  vector<ControlPotmeter*> potmeters;
};

#endif
