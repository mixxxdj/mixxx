#ifndef MIDIOBJECT_H
#define MIDIOBJECT_H

#include <stdio.h>
#include <stdlib.h>

#ifdef __PORTMIDI__
  #include "portmidi.h"
#endif
#ifdef __ALSAMIDI__
  #include <sys/asoundlib.h>
#endif
#ifdef __OSSMIDI__
  #include <unistd.h>
  #include <fcntl.h>
  #include <stdio.h>
#endif

#include <vector>
#include <qthread.h>
#include <qobject.h>
#include "defs.h"
#include "configobject.h"

class ControlObject;

class MidiObject : public QThread  {
//  Q_OBJECT
public:
  MidiObject(ConfigObject *c);
  ~MidiObject();
  void add(ControlObject* c);
  void remove(ControlObject* c);
protected:
#ifdef __PORTMIDI__
  PmEvent buffer[2];
  PmStream *midi;
#endif
#ifdef __ALSAMIDI__
  snd_rawmidi_t *handle;
  char *buffer;
#endif
#ifdef __OSSMIDI__
  int handle;
  char *buffer;
#endif
  void run();

  static ConfigObject *config;
  int fd, count, size, no;
  std::vector<ControlObject*> controlList;
};

#endif


