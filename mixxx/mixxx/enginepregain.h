#ifndef ENGINEPREGAIN_H
#define ENGINEPREGAIN_H

#include <qobject.h>

#include "engineobject.h"
#include "midiobject.h"
#include "controlpotmeter.h"

class EnginePregain : public EngineObject {
  Q_OBJECT
private:
 public:
  ControlPotmeter* pregainpot;
  FLOAT pregain;
  EnginePregain(int, MidiObject*);
  ~EnginePregain();
  CSAMPLE *process(CSAMPLE*, int);
 public slots:
  void slotUpdate(FLOAT);

 private:
  CSAMPLE *buffer;
};

#endif
