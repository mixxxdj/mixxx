#include "enginedelay.h"

/*----------------------------------------------------------------

  ----------------------------------------------------------------*/
EngineDelay::EngineDelay(WKnob *DialDelay)
{
    process_buffer = new CSAMPLE[MAX_BUFFER_LEN];
    delay_buffer = new CSAMPLE[max_delay];

    delay_pos = 0;

    potmeter = new ControlPotmeter(ConfigKey("void", "delay"), 0, max_delay);
    connect(potmeter, SIGNAL(valueChanged(FLOAT_TYPE)), this, SLOT(slotUpdate(FLOAT_TYPE)));
    connect(DialDelay, SIGNAL(valueChanged(int)), potmeter, SLOT(slotSetPosition(int))); 
    potmeter->slotSetPosition(0);
}

EngineDelay::~EngineDelay()
{
    delete potmeter;
    delete [] process_buffer;
    delete [] delay_buffer;
}

void EngineDelay::slotUpdate(FLOAT_TYPE newvalue)
{
    delay =(int) newvalue;
    qDebug("Delay: %d samples", delay);
}

CSAMPLE *EngineDelay::process(const CSAMPLE *source, const int buffer_size)
{
    int delay_source_pos = (delay_pos+max_delay-delay)%max_delay;
    if ((delay_source_pos<0) || (delay_source_pos>max_delay)) 
				qFatal("Error in EngineDelay: delay_source_pos = %d", delay_source_pos);

    for (int i=0; i<buffer_size; i++) {
	// put sample into delay buffer:
	delay_buffer[delay_pos] = source[i];
	delay_pos++;
	if (delay_pos >= max_delay) delay_pos=0;
 	// Take "old" sample from delay buffer and mix it with the source buffer:
        process_buffer[i] = 0.5*(delay_buffer[delay_source_pos] + source[i]);
	delay_source_pos++;
	if (delay_source_pos >= max_delay) delay_source_pos=0;
    }
    return process_buffer;
}

