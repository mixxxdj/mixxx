#ifndef ENGINEIIRFILTER_H
#define ENGINEIIRFILTER_H

#include <qobject.h>

#include "engineobject.h"
#include "midiobject.h"
#include "controlpotmeter.h"
#include "controlpushbutton.h"

class EngineIIRfilter : public EngineObject {
 Q_OBJECT
 private:
  ControlPushButton* killbutton;
  ControlPotmeter* filterpot;
  double *coefs;
  static const int NZEROS = 8;
  static const int NPOLES = 8;
  double xv[NZEROS+1], yv[NPOLES+1];
 public:
  FLOAT gain;
  EngineIIRfilter(int, int , int , MidiObject*, double *);
  ~EngineIIRfilter();
  void process(CSAMPLE*, CSAMPLE*, int);
 public slots:
  void slotUpdate();
};

//
// Defines filter coefficients for IIR filters:
//

// corner at 600 Hz
static const double bessel_lowpass[13] = { 7.444032197e+08,
					    8, 28, 56, 70,
					    -0.3800297563, 3.4120798629, 
					    -13.4230504610, 30.2214248640,
					    -42.5938048390, 38.4826057150,
					    -21.7665031930, 7.0472774638};

// corner at 4000 Hz:
static const double bessel_highpass[13] = {2.465837728e+00, // gain
					   - 8,+ 28, - 56, + 70,
					   -0.1552424571, 1.5489970216,
					   -6.7821376632,17.0223182510,
					   -26.7923322400,27.0856195480,
					   -17.1796384890, 6.2523870250};

// 4th order bandpass at 600 - 4000Hz:
static const double bessel_bandpass[13] = {1.455078491e+02,
					   0,-4,0,6,
					   -0.1002852333, 1.0213655417,
					   -4.6272090652,  12.1726925480,
					   -20.3120761830, 21.9557125490,
					   -14.9560287020,  5.8458265249};

#endif
