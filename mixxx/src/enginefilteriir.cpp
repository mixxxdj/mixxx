#include "enginefilteriir.h"

EngineFilterIIR::EngineFilterIIR(const double *_coefs)
{
    coefs = _coefs;
    buffer = new CSAMPLE[MAX_BUFFER_LEN];

    // Reset the yv's:
    for (int i=0; i<NPOLES; i++)
        yv[i]=xv[i]=0;
}

EngineFilterIIR::~EngineFilterIIR() {
  delete [] buffer;
}

CSAMPLE *EngineFilterIIR::process(const CSAMPLE *source, const int buf_size) {
  double GAIN =  coefs[0];
  for (int i=0; i<buf_size; i++) {
      xv[0] = xv[1]; xv[1] = xv[2]; xv[2] = xv[3]; xv[3] = xv[4];
	  xv[4] = xv[5]; xv[5] = xv[6]; xv[6] = xv[7]; xv[7] = xv[8]; 
	  xv[8] = source[i]/GAIN;
	  yv[0] = yv[1]; yv[1] = yv[2]; yv[2] = yv[3]; yv[3] = yv[4];
	  yv[4] = yv[5]; yv[5] = yv[6]; yv[6] = yv[7]; yv[7] = yv[8]; 
	  
	  yv[8] =   (xv[0] + xv[8]) + coefs[1] * (xv[1] + xv[7]) + 
	      coefs[2] * (xv[2] + xv[6])
	      + coefs[3] * (xv[3] + xv[5]) + coefs[4] * xv[4]
	      + (coefs[5] * yv[0]) + ( coefs[6] * yv[1])
	      + (coefs[7] * yv[2]) + ( coefs[8] * yv[3])
	      + (coefs[9] * yv[4]) + ( coefs[10] * yv[5])
	      + (coefs[11] * yv[6]) + ( coefs[12] * yv[7]);
	  //if (!(yv[8]<100000 || yv[8]>-100000))
	  //    qDebug("Overflow in engineiirfilter: %f %f %f.", source[i], yv[8], gain);
	  buffer[i] = yv[8];
  }

  // Check for denormals
  {for (int i=0; i<9; i++)
  {
     xv[i] = zap_denormal(xv[i]);
     yv[i] = zap_denormal(yv[i]);
  }}

  return buffer;
}
