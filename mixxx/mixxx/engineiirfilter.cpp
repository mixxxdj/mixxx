#include "engineiirfilter.h"

EngineIIRfilter::EngineIIRfilter(int potmeter_midi, int button_midi,
				 int button_bit, MidiObject *midi, const double *_coefs) {
  //  Initialize the buttons:
  //killbutton = new ControlPushButton("kill", simulated_latching, button_midi,
  //				     button_bit, midi);
  //connect(killbutton, SIGNAL(valueChanged(valueType)), this, SLOT(slotUpdate()));

  filterpot = new ControlPotmeter("filterpot", potmeter_midi, midi,0,3);
  connect(filterpot, SIGNAL(valueChanged(FLOAT)), this, SLOT(slotUpdate()));
  coefs = _coefs;
  buffer = new CSAMPLE[MAX_BUFFER_LEN];
  gain = 1;
  // Reset the yv's:
  for (int i=0; i<8; i++) yv[i]=0;
}

EngineIIRfilter::~EngineIIRfilter() {
  delete killbutton;
  delete filterpot;
  delete [] buffer;
}

CSAMPLE *EngineIIRfilter::process(CSAMPLE *source, int buf_size) {
  double GAIN =  coefs[0];
  for (int i=0; i<buf_size; i++) {
    xv[0] = xv[1]; xv[1] = xv[2]; xv[2] = xv[3]; xv[3] = xv[4];
    xv[4] = xv[5]; xv[5] = xv[6]; xv[6] = xv[7]; xv[7] = xv[8]; 
    xv[8] = source[i] / GAIN;
    yv[0] = yv[1]; yv[1] = yv[2]; yv[2] = yv[3]; yv[3] = yv[4];
    yv[4] = yv[5]; yv[5] = yv[6]; yv[6] = yv[7]; yv[7] = yv[8]; 

    yv[8] =   (xv[0] + xv[8]) + coefs[1] * (xv[1] + xv[7]) + 
	coefs[2] * (xv[2] + xv[6])
	+ coefs[3] * (xv[3] + xv[5]) + coefs[4] * xv[4]
	+ (coefs[5] * yv[0]) + ( coefs[6] * yv[1])
	+ (coefs[7] * yv[2]) + ( coefs[8] * yv[3])
	+ (coefs[9] * yv[4]) + ( coefs[10] * yv[5])
	+ (coefs[11] * yv[6]) + ( coefs[12] * yv[7]);
    
    //qDebug("%f %f",source[i],gain*yv[8]);
    buffer[i] = gain*yv[8];
  }
  return buffer;
}

void EngineIIRfilter::slotUpdate() {
  // We've been called when either the killbutton or the potmeter has
  // been touched. We have to check both.
  //if (killbutton->getValue()==down)
  //  gain = 0;
  //else
  gain = filterpot->getValue();
  //qDebug("IIRfilter gain: %f",gain);
}

