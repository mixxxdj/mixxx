#ifndef MIDIOBJECT_H
#define MIDIOBJECT_H

#include <stdio.h>
#include <stdlib.h>

#ifdef __PORTMIDI__
  #include "portmidi.h"
#else
  #include <sys/asoundlib.h>
#endif

#include <vector>
#include <qthread.h>
#include <qobject.h>
#include "defs.h"
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
#ifdef __PORTMIDI__
  PmEvent buffer[2];
  PmStream *midi;
#else
  snd_rawmidi_t *handle;
  char *buffer;
#endif
  void run();
  int fd, count, size, no_potmeters, no_buttons;
  std::vector<ControlPushButton*> buttons;
  std::vector<ControlPotmeter*> potmeters;
};

#endif


