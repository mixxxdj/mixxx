#include "engineflanger.h"
#include "qradiobutton.h"

/*----------------------------------------------------------------
  A flanger effect.
  The flanger is controlled by the following variables:
    average_delay_length - The average length of the delay, which is modulated by the LFO.
    LFOperiod - the period of LFO given in samples.
    LFOamplitude - the amplitude of the modulation of the delay length.
    depth - the depth of the flanger, controlled by a ControlPotmeter.
  ----------------------------------------------------------------*/
EngineFlanger::EngineFlanger(DlgFlanger *_dlg, const char *group)
{
    dlg = _dlg;

    // Fixed values of controls:
    LFOperiod = 40000;
    LFOamplitude = 240;
    average_delay_length = 250;

    // Init. buffers:
    process_buffer = new CSAMPLE[MAX_BUFFER_LEN];
    delay_buffer = new CSAMPLE[max_delay];

    // Init. potmeters
    potmeterDepth = new ControlPotmeter(ConfigKey(group, "depth"), 0., 1.);
    connect(dlg->DialDepth, SIGNAL(valueChanged(int)), potmeterDepth, SLOT(slotSetPosition(int))); 
    connect(potmeterDepth, SIGNAL(valueChanged(FLOAT_TYPE)), this, SLOT(slotUpdateDepth(FLOAT_TYPE)));

    potmeterDelay = new ControlPotmeter(ConfigKey(group, "delay"), 50, 1000);
    connect(dlg->DialDelay, SIGNAL(valueChanged(int)), potmeterDelay, SLOT(slotSetPosition(int))); 
    connect(potmeterDelay, SIGNAL(valueChanged(FLOAT_TYPE)), this, SLOT(slotUpdateDelay(FLOAT_TYPE)));

    potmeterLFOperiod = new ControlPotmeter(ConfigKey(group, "LFO period"), 5000, 80000);
    connect(dlg->DialPeriod, SIGNAL(valueChanged(int)), potmeterLFOperiod, SLOT(slotSetPosition(int))); 
    connect(potmeterLFOperiod, SIGNAL(valueChanged(FLOAT_TYPE)), this, SLOT(slotUpdateLFOperiod(FLOAT_TYPE)));

    // Init. channel selects:
    pushbuttonChannelA = new ControlPushButton( ConfigKey(group, "channel A"), simulated_latching, 
						dlg->BulbChannelA);
    pushbuttonChannelB = new ControlPushButton( ConfigKey(group, "channel B"), simulated_latching, 
						dlg->BulbChannelB);
    connect(dlg->PushButtonChannelA, SIGNAL(pressed()), pushbuttonChannelA, SLOT(pressed()));
    connect(dlg->PushButtonChannelA, SIGNAL(released()), pushbuttonChannelA, SLOT(released()));
    connect(pushbuttonChannelA, SIGNAL(valueChanged(valueType)), this, SLOT(slotUpdateChannelSelectA(valueType)));
    connect(dlg->PushButtonChannelB, SIGNAL(pressed()), pushbuttonChannelB, SLOT(pressed()));
    connect(dlg->PushButtonChannelB, SIGNAL(released()), pushbuttonChannelB, SLOT(released()));
    connect(pushbuttonChannelB, SIGNAL(valueChanged(valueType)), this, SLOT(slotUpdateChannelSelectB(valueType)));
    channel_A = channel_B = off;

EngineFlanger::~EngineFlanger()
{
    delete potmeterDepth;
    delete potmeterDelay;
    delete potmeterLFOperiod;
    delete pushbuttonChannelA;
    delete pushbuttonChannelB;
    delete [] process_buffer;
    delete [] delay_buffer;
}

void EngineFlanger::slotUpdateDepth(FLOAT_TYPE newvalue) 
{
    depth = newvalue;
}

void EngineFlanger::slotUpdateDelay(FLOAT_TYPE newvalue) 
{
    average_delay_length = (int)newvalue;
    LFOamplitude = (int)(0.9*average_delay_length);
}

void EngineFlanger::slotUpdateLFOperiod(FLOAT_TYPE newvalue) 
{
    LFOperiod = (int) newvalue;
}

void EngineFlanger::slotUpdateChannelSelectA(valueType newvalue) 
{
    channel_A = newvalue;
    if (channel_A)  
    {
	channel_B = off;
	pushbuttonChannelB->setValue(off);
    } 
}

void EngineFlanger::slotUpdateChannelSelectB(valueType newvalue) 
{
    channel_B = newvalue;
    if (channel_B) 
    {
	channel_A = off;
	pushbuttonChannelA->setValue(off);
    }
}

CSAMPLE *EngineFlanger::process(const CSAMPLE *source, const int buffer_size)
{
    CSAMPLE delayed_sample,prev,next;
    FLOAT_TYPE frac;

    for (int i=0; i<buffer_size; i++) 
    {
	// put sample into delay buffer:
	delay_buffer[delay_pos] = source[i];
	delay_pos++;
	if (delay_pos >= max_delay) delay_pos=0;
	// Update the LFO to find the current delay:
	time++;
	if (time==LFOperiod) time=0;
	delay = average_delay_length + LFOamplitude*sin( 2*pi*((FLOAT_TYPE) time)/((FLOAT_TYPE) LFOperiod) );
	// Make a linear interpolation to find the delayed sample:
	prev = delay_buffer[(delay_pos-(int)delay+max_delay-1) % max_delay];
	next = delay_buffer[(delay_pos-(int)delay+max_delay) % max_delay];
	frac = delay - floor(delay);
	delayed_sample = prev + frac*(next-prev);
 	// Take the sample from the delay buffer and mix it with the source buffer:
        process_buffer[i] = source[i] + depth*delayed_sample;
    }
    return process_buffer;
}

