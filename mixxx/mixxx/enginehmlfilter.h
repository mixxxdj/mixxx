#ifndef ENGINEHMLFILTER_H
#define ENGINEHMLFILTER_H

#include <qobject.h>

#include "engineobject.h"
#include "midiobject.h"
#include "controllogpotmeter.h"
#include "controlpushbutton.h"
#include "engineiirfilter.h"

class EngineHMLfilter : public EngineObject {
  Q_OBJECT
 private:
  ControlPushButton* killbutton;
  ControlLogpotmeter *highfilterpot, *midfilterpot, *lowfilterpot;
  double *low_coefs, *high_coefs;
  static const int NZEROS = 8;
  static const int NPOLES = 8;
  double xv_low[NZEROS+1], yv_low[NPOLES+1], xv_high[NZEROS+1], yv_high[NPOLES+1];
 public:
  FLOAT highgain, midgain, lowgain;
  EngineHMLfilter(int, int , int , MidiObject*, double *, double *);
  ~EngineHMLfilter();
  void process(CSAMPLE*, CSAMPLE*, int);
 public slots:
  void slotUpdate();
};

#endif
