#include "enginevolume.h"

/*----------------------------------------------------------------
  Volume effect.
  ----------------------------------------------------------------*/
EngineVolume::EngineVolume(const char *group)
{
    potmeter = new ControlLogpotmeter(new ConfigKey(group, "volume"), 5.0);
    volume = 1.0;
    buffer = new CSAMPLE[MAX_BUFFER_LEN];

    connect(potmeter, SIGNAL(valueChanged(FLOAT_TYPE)), this, SLOT(slotUpdate(FLOAT_TYPE)));
}

EngineVolume::~EngineVolume()
{
    delete potmeter;
    delete [] buffer;
}

void EngineVolume::slotUpdate(FLOAT_TYPE newvalue)
{
    volume = newvalue;
    //qDebug("Volume: %f",volume);
}

CSAMPLE *EngineVolume::process(const CSAMPLE *source, const int buffer_size)
{
    for (int i=0; i<buffer_size; i++)
        buffer[i] = source[i]*volume;
    return buffer;
}
