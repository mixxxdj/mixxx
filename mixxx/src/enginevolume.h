#ifndef ENGINEVOLUME_H
#define ENGINEVOLUME_H

#include <qobject.h>

#include "engineobject.h"
#include "midiobject.h"
#include "controllogpotmeter.h"

class EngineVolume : public EngineObject {
  Q_OBJECT
public:
  EngineVolume(const char *group);
  ~EngineVolume();
  CSAMPLE *process(const CSAMPLE*, const int);

  ControlLogpotmeter* potmeter;
public slots:
  void slotUpdate(FLOAT_TYPE);

private:
  CSAMPLE *buffer;
  FLOAT_TYPE volume;
};

#endif
