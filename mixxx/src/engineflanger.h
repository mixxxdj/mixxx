#ifndef ENGINEFLANGER_H
#define ENGINEFLANGER_H

#include <qobject.h>

#include "engineobject.h"
#include "controlpotmeter.h"
#include "wknob.h"
#include "dlgflanger.h"

class EngineFlanger : public EngineObject {
  Q_OBJECT
public:
  EngineFlanger(DlgFlanger *, const char *);
  ~EngineFlanger();
  CSAMPLE *process(const CSAMPLE *, const int);
  ControlPotmeter *potmeterDepth, *potmeterDelay, *potmeterLFOperiod;
  bool channel_A, channel_B;

public slots:
  void slotUpdateDepth(FLOAT_TYPE);
  void slotUpdateDelay(FLOAT_TYPE);
  void slotUpdateLFOperiod(FLOAT_TYPE);
  void slotUpdateChannelSelectA(bool); 
  void slotUpdateChannelSelectB(bool); 

private:
  static const int max_delay = 5000;  
  CSAMPLE *process_buffer, *delay_buffer;
  FLOAT_TYPE depth; // the depth of the flanger [0..1];
  int  LFOamplitude;
  int average_delay_length;
  int LFOperiod; // Period of the LFO measured in samples
  int time;
  FLOAT_TYPE delay;
  int delay_pos;
  DlgFlanger *dlg;

};

#endif
