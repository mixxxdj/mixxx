#include "midiobject.h"
#include "configobject.h"
#include "controlobject.h"
#include "controlpushbutton.h"
#include <algorithm>

// Static member variable definition
ConfigObject *MidiObject::config = 0;

/* -------- ------------------------------------------------------
   Purpose: Initialize midi, and start parsing loop
   Input:   None. Automatically selects default midi input sound
            card and device.
   Output:  -
   -------- ------------------------------------------------------ */
MidiObject::MidiObject(ConfigObject *c)
{
  config = c;
  no = 0;

#ifdef __PORTMIDI__
  // Open midi device for input
  Pm_Initialize();

  /*for (i = 0; i < Pm_CountDevices(); i++) {
	    const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
        printf("%d: %s, %s", i, info->interf, info->name);
        if (info->input) printf(" (input)");
        if (info->output) printf(" (output)");
        printf("\n");
  }*/

  PmError err = Pm_OpenInput(&midi, 1, NULL, 100, NULL, NULL, NULL);
  if (err) {
        qDebug("could not open midi device: %s\n", Pm_GetErrorText(err));
  }

  // Start the midi thread:
  start();
#endif
#ifdef __ALSAMIDI__
  // Open midi device for input
  int card = snd_defaults_rawmidi_card();
  int device = snd_defaults_rawmidi_device();
  int err;
  if ((err = snd_rawmidi_open(&handle, card, device, SND_RAWMIDI_OPEN_INPUT)) != 0)
  {
      qDebug("Open of midi failed: %s.", snd_strerror(err));
  }
  else
  {
      // Allocate buffer
      buffer = new char[4096];
      if (buffer == 0)
      {
          qDebug("Midi: Error allocating buffer");
          return;
      }

      // Set number of bytes received, before snd_rawmidi_read is woken up.
      snd_rawmidi_params_t params;
      params.channel = SND_RAWMIDI_CHANNEL_INPUT;
      params.size    = 4096;
      params.min     = 1;
      err = snd_rawmidi_channel_params(handle,&params);

      // Start the midi thread:
      start();
  }
#endif
#ifdef __OSSMIDI__
  // Allocate buffer
  buffer = new char[4096];
  if (buffer == 0)
  {
    qDebug("Midi: Error allocating buffer");
    return;
  }
  // Open midi device
  handle = open("/dev/midi",0);
  if (handle == -1)
  {
    qDebug("Open of /dev/midi failed.");
    return;
  }
  start();
#endif

};

/* -------- ------------------------------------------------------
  Purpose: Deallocates midi buffer, and closes device
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
MidiObject::~MidiObject() {
#ifdef __PORTMIDI__
  // Close device
  Pm_Close(midi);
#endif
#ifdef __ALSAMIDI__
  // Close device
  snd_rawmidi_close(handle);
  // Deallocate buffer
  delete [] buffer;
#endif
#ifdef __OSSMIDI__
  close(handle);
  delete [] buffer;
#endif
};

/* -------- ------------------------------------------------------
   Purpose: Add a control ready to recieve midi events.
   Input:   a pointer to the control. The second argument
            is the method in the control to call when the button
	    has been moved.
   Output:  -
   -------- ------------------------------------------------------ */
void MidiObject::add(ControlObject* c)
{
    controlList.push_back(c);
    no++;
    qDebug("Registered midi control %s (%p).", c->print()->ascii(),c);
}

void MidiObject::remove(ControlObject* c)
{
    std::vector<ControlObject*>::iterator it =
        std::find(controlList.begin(), controlList.end(), c);
    if (it != controlList.end())
    {
        controlList.erase(it);
        no--;
    }
    else
        qWarning("Control which is requested for removal in MidiObject does not exist.");
}

/* -------- ------------------------------------------------------
   Purpose: Loop for parsing midi events
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
void MidiObject::run()
{
    int stop = 0;
#ifdef __PORTMIDI__
    PmError err;
    char channel, midicontrol, midivalue;
#endif

    while(stop == 0)
	{



#ifdef __PORTMIDI__
        err = Pm_Poll(midi);
        if (err == TRUE)
        {
            if (Pm_Read(midi, buffer, 1) > 0)
            {
                midicontrol = Pm_MessageData1(buffer[0].message);
                midivalue = Pm_MessageData2(buffer[0].message);
            } else {
                qDebug("Error in Pm_Read: %s\n", Pm_GetErrorText(err));
                break;
            }
        } else if (err != FALSE) {
            qDebug("Error in Pm_Poll: %s\n", Pm_GetErrorText(err));
            break;
        }
#else
        /*
        First read until we get a midi channel event:
        */
#ifdef __ALSAMIDI__
        do
		{
            int no = snd_rawmidi_read(handle,&buffer[0],1);
            if (no != 1)
                qWarning("Warning: midiobject recieved %i bytes.", no);
        } while (buffer[0] & 128 != 128);
#endif
#ifdef __OSSMIDI__
        do
		{
            int no = read(handle,&buffer[0],1);
            //qDebug("midi: %i",(short int)buffer[0]);
            if (no != 1)
                qWarning("Warning: midiobject recieved %i bytes.", no);
        } while (buffer[0] & 128 != 128); // Continue until we receive a status byte (bit 7 is set)
#endif
        /*
        and then get the following 2 bytes:
        */
        char channel;
        char midicontrol;
        char midivalue;
#ifdef __ALSAMIDI__
        for (int i=1; i<3; i++)
        {
            int no = snd_rawmidi_read(handle,&buffer[i],1);
            if (no != 1)
                qWarning("Warning: midiobject recieved %i bytes.", no);
        }
        channel = buffer[0] & 15;
        midicontrol = buffer[1];
        midivalue = buffer[2];
#endif
#ifdef __OSSMIDI__
        for (int i=1; i<3; i++)
        {
            int no = read(handle,&buffer[i],1);
            if (no != 1)
                qWarning("Warning: midiobject recieved %i bytes.", no);
        }
        channel = buffer[0] & 15; // The channel is store in the lower 4 bits of the status byte received
        midicontrol = buffer[1];
        midivalue = buffer[2];
#endif

#endif

        //qDebug("Received midi message: ch %i no %i val %i",(int)channel,(int)midicontrol,(int)midivalue);

        // Check the potmeters:
        for (int i=0; i<no; i++)
        {
            //qDebug("(%i) checking: no %i ch %i",i,(int)controlList[i]->cfgOption->val->midino,(int)controlList[i]->cfgOption->val->midichannel);
            if (controlList[i]->cfgOption->val->midino == midicontrol &
                controlList[i]->cfgOption->val->midichannel == channel)
            {
		//qDebug("gotit");
                // Check for possible bit mask
                int midimask = controlList[i]->cfgOption->val->midimask;
                if (midimask > 0)
                    controlList[i]->slotSetPosition((int)(midimask & midivalue));
                else
                    controlList[i]->slotSetPositionMidi((int)midivalue); // 127-midivalue

                break;
            }
        }
    }
};
