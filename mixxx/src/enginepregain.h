#ifndef ENGINEPREGAIN_H
#define ENGINEPREGAIN_H

#include <qobject.h>

#include "engineobject.h"
#include "midiobject.h"
#include "controllogpotmeter.h"

class EnginePregain : public EngineObject {
  Q_OBJECT
public:
  EnginePregain(const char *group);
  ~EnginePregain();
  CSAMPLE *process(const CSAMPLE*, const int);

  ControlLogpotmeter* potmeter;
public slots:
  void slotUpdate(FLOAT_TYPE);

private:
  CSAMPLE *buffer;
  FLOAT_TYPE pregain;
};

#endif
