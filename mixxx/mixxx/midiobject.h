#if !defined(__midi_h)
#define __midi_h

#include <sys/asoundlib.h>
#include <semaphore.h>
#include <stdlib.h>
#include <vector>

#include <qthread.h>

#include "defs.h"
#include "controlpushbutton.h"
#include "controlobject.h"
#include "controlpotmeter.h"

class ControlPushButton;
class ControlPotmeter;

class MidiObject : public QThread {
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
// private:
//  pthread_t midi_thread;
};

//void *ThreadStartup(void *_tgtObject);

#endif
