#include "enginepregain.h"

/*----------------------------------------------------------------
  A pregaincontrol is ... a pregain.
  ----------------------------------------------------------------*/
EnginePregain::EnginePregain(int potmeter_midi, MidiObject* midi)
{
  pregainpot = new ControlPotmeter("pregainpot", potmeter_midi, midi, 5., 0.25);
  connect(pregainpot, SIGNAL(valueChanged(FLOAT)), this, SLOT(slotUpdate(FLOAT)));
  pregain = 1.0;
  buffer = new CSAMPLE[MAX_BUFFER_LEN];
}

EnginePregain::~EnginePregain() {
    delete pregainpot;
	delete [] buffer;
}

void EnginePregain::slotUpdate(FLOAT newvalue) {
  pregain = newvalue;
}

CSAMPLE *EnginePregain::process(CSAMPLE *source, int buffer_size) {
  for (int i=0; i<buffer_size; i++)
    buffer[i] = source[i]*pregain;
  return buffer;
}
