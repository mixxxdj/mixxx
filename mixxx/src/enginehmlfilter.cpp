#include "enginehmlfilter.h"

EngineHMLfilter::EngineHMLfilter(int high_potmeter_midi, int mid_potmeter_midi, int low_potmeter_midi,
				 MidiObject *midi, const double *_high_coefs, const double *_low_coefs) {
  //  Initialize the buttons:
  highfilterpot = new ControlLogpotmeter("filterpot", high_potmeter_midi, midi);
  midfilterpot = new ControlLogpotmeter("filterpot", mid_potmeter_midi, midi);
  lowfilterpot = new ControlLogpotmeter("filterpot", low_potmeter_midi, midi);
  connect(highfilterpot, SIGNAL(valueChanged(FLOAT)), this, SLOT(slotUpdate()));
  connect(midfilterpot, SIGNAL(valueChanged(FLOAT)), this, SLOT(slotUpdate()));
  connect(lowfilterpot, SIGNAL(valueChanged(FLOAT)), this, SLOT(slotUpdate()));

  high_coefs = _high_coefs;
  low_coefs = _low_coefs;
}

EngineHMLfilter::~EngineHMLfilter() {
  delete highfilterpot;
  delete midfilterpot;
  delete lowfilterpot;
}

CSAMPLE *EngineHMLfilter::process(CSAMPLE *source, int buf_size) {
  for (int i=0; i<buf_size; i++) {
    /*
      highpass filter:
    */
    xv_high[0] = xv_high[1]; xv_high[1] = xv_high[2]; xv_high[2] = xv_high[3]; xv_high[3] = xv_high[4];
    xv_high[4] = xv_high[5]; xv_high[5] = xv_high[6]; xv_high[6] = xv_high[7]; xv_high[7] = xv_high[8]; 
    xv_high[8] = source[i] / high_coefs[0];
    yv_high[0] = yv_high[1]; yv_high[1] = yv_high[2]; yv_high[2] = yv_high[3]; yv_high[3] = yv_high[4];
    yv_high[4] = yv_high[5]; yv_high[5] = yv_high[6]; yv_high[6] = yv_high[7]; yv_high[7] = yv_high[8]; 

    yv_high[8] =   (xv_high[0] + xv_high[8]) + high_coefs[1] * (xv_high[1] + xv_high[7]) + 
      high_coefs[2] * (xv_high[2] + xv_high[6])
      + high_coefs[3] * (xv_high[3] + xv_high[5]) + high_coefs[4] * xv_high[4]
      + (high_coefs[5] * yv_high[0]) + ( high_coefs[6] * yv_high[1])
      + (high_coefs[7] * yv_high[2]) + ( high_coefs[8] * yv_high[3])
      + (high_coefs[9] * yv_high[4]) + ( high_coefs[10] * yv_high[5])
      + (high_coefs[11] * yv_high[6]) + ( high_coefs[12] * yv_high[7]);
    /*
      lowpass filter:
    */
    xv_low[0] = xv_low[1]; xv_low[1] = xv_low[2]; xv_low[2] = xv_low[3]; xv_low[3] = xv_low[4];
    xv_low[4] = xv_low[5]; xv_low[5] = xv_low[6]; xv_low[6] = xv_low[7]; xv_low[7] = xv_low[8]; 
    xv_low[8] = source[i] / low_coefs[0];
    yv_low[0] = yv_low[1]; yv_low[1] = yv_low[2]; yv_low[2] = yv_low[3]; yv_low[3] = yv_low[4];
    yv_low[4] = yv_low[5]; yv_low[5] = yv_low[6]; yv_low[6] = yv_low[7]; yv_low[7] = yv_low[8]; 

    yv_low[8] =   (xv_low[0] + xv_low[8]) + low_coefs[1] * (xv_low[1] + xv_low[7]) + 
      low_coefs[2] * (xv_low[2] + xv_low[6])
      + low_coefs[3] * (xv_low[3] + xv_low[5]) + low_coefs[4] * xv_low[4]
      + (low_coefs[5] * yv_low[0]) + ( low_coefs[6] * yv_low[1])
      + (low_coefs[7] * yv_low[2]) + ( low_coefs[8] * yv_low[3])
      + (low_coefs[9] * yv_low[4]) + ( low_coefs[10] * yv_low[5])
      + (low_coefs[11] * yv_low[6]) + ( low_coefs[12] * yv_low[7]);
    /*
      Mix the filters together:
    */
    //destination[i] = (highgain-midgain)*yv_high[8] + (lowgain-midgain)*yv_low[8];
    //destination[i] += highgain*yv_high[8] + lowgain*yv_low[8];
  }
  return 0; //****************
}

void EngineHMLfilter::slotUpdate() {
  highgain = highfilterpot->getValue();
  midgain = midfilterpot->getValue();
  lowgain = lowfilterpot->getValue();
  //qDebug("filtergains: %g %g %g",highgain,midgain,lowgain);
}
