#include "enginepregain.h"

/*----------------------------------------------------------------
  A pregaincontrol is ... a pregain.
  ----------------------------------------------------------------*/
EnginePregain::EnginePregain(int potmeter_midi, MidiObject* midi)
{
  pregainpot = new ControlPotmeter("pregainpot", potmeter_midi, midi, 5., 0.25);
  connect(pregainpot, SIGNAL(valueChanged(FLOAT)), this, SLOT(slotUpdate(FLOAT)));
  pregain = 1.0;
}

EnginePregain::~EnginePregain() {
    delete pregainpot;
}

void EnginePregain::slotUpdate(FLOAT newvalue) {
  pregain = newvalue;
}

void EnginePregain::process(CSAMPLE *source, CSAMPLE* destination, int buffer_size) {
  for (int i=0; i<buffer_size; i++)
    destination[i] = source[i]*pregain;
}
