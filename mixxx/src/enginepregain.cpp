#include "enginepregain.h"

/*----------------------------------------------------------------
  A pregaincontrol is ... a pregain.
  ----------------------------------------------------------------*/
EnginePregain::EnginePregain(const char *group)
{
    potmeter = new ControlLogpotmeter(new ConfigObject::ConfigKey(group, "pregain"), 5.0);
    pregain = 1.0;
    buffer = new CSAMPLE[MAX_BUFFER_LEN];

    connect(potmeter, SIGNAL(valueChanged(FLOAT_TYPE)), this, SLOT(slotUpdate(FLOAT_TYPE)));
}

EnginePregain::~EnginePregain()
{
    delete potmeter;
    delete [] buffer;
}

void EnginePregain::slotUpdate(FLOAT_TYPE newvalue)
{
    pregain = newvalue;
    qDebug("Pregain: %f",pregain);
}

CSAMPLE *EnginePregain::process(const CSAMPLE *source, const int buffer_size)
{
    for (int i=0; i<buffer_size; i++)
        buffer[i] = source[i]*pregain;
    return buffer;
}
