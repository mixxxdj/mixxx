#include "midiobject.h"

/* -------- ------------------------------------------------------
   Purpose: Initialize midi, and start parsing loop
   Input:   None. Automatically selects default midi input sound
            card and device.
   Output:  -
   -------- ------------------------------------------------------ */
MidiObject::MidiObject() {
  // Init buttons:
  no_potmeters = 0;
  no_buttons = 0;

  // Open midi device for input
  int card = snd_defaults_rawmidi_card();
  int device = snd_defaults_rawmidi_device();
  int err;
  if ((err = snd_rawmidi_open(&handle, card, device, SND_RAWMIDI_OPEN_INPUT)) != 0) {
    qDebug("Open of midi failed: %s.", snd_strerror(err));
  }  else {

    // Allocate buffer
    buffer = new char[4096];
    if (buffer == 0) {
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
};

/* -------- ------------------------------------------------------
  Purpose: Deallocates midi buffer, and closes device
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
MidiObject::~MidiObject() {
  // Close device
  snd_rawmidi_close(handle);

  // Deallocate buffer
  delete [] buffer;
};

/* -------- ------------------------------------------------------
   Purpose: Add a button ready to recieve midi events.
   Input:   a pointer to the button. The second argument
            is the method in the control to call when the button
	    has been moved.
   Output:  -
   -------- ------------------------------------------------------ */
void MidiObject::addbutton(ControlPushButton* newbutton) {
  buttons.push_back(newbutton);
  no_buttons++;
  qDebug("Registered midi button %s.", newbutton->print());
}

void MidiObject::removebutton(ControlPushButton* button) {
  //for (iterator i=0; i<buttons.size(); i++)
  //  if (buttons[i] == button) buttons->erase(i);
  qWarning("Removal of midibutton not implemented");
}

/* -------- ------------------------------------------------------
   Purpose: Add a potmeter ready to recieve midi events.
   Input:   a pointer to the potmeter. The second argument
            is the method in the control to call when the potmeter
	    has been moved.
   Output:  -
   -------- ------------------------------------------------------ */
void MidiObject::addpotmeter(ControlPotmeter* newpotmeter) {
  potmeters.push_back(newpotmeter);
  no_potmeters++;
  qDebug("Registered midi potmeter %s.", newpotmeter->print());;
}

void MidiObject::removepotmeter(ControlPotmeter* potmeter) {
  vector<ControlPotmeter*>::iterator iter = 
    find(potmeters.begin(), potmeters.end(), potmeter);
  if (iter != potmeters.end())
    potmeters.erase(iter);
  else
    qWarning("Potmeter which is requested for removal in MidiObject does not exist.");
  //for (int i=0; i<potmeters.size(); i++)
  //  if (potmeters[i] == potmeter) potmeters.erase(i);
  //qWarning("Remove of midi potmeters not yet implemented.");
}

/* -------- ------------------------------------------------------
   Purpose: Loop for parsing midi events
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
void MidiObject::run() {
  int stop = 0;
  while(stop == 0) {
    /*
      First read until we get at -79 event:
    */
    do {
      int no = snd_rawmidi_read(handle,&buffer[0],1);
      if (no != 1)
	std::cout << "Warning: midi recieved " << no << " bytes.\n";
    } while (buffer[0] != -79);
    /*
      and then get the following 2 bytes:
    */
    for (int i=1; i<3; i++) {
      int no = snd_rawmidi_read(handle,&buffer[i],1);
      if (no != 1)
	std::cout << "Warning: midi recieved " << no << " bytes.\n";
    }
    
    char channel = buffer[0];
    char midicontrol = buffer[1];
    char midivalue = buffer[2];

    qDebug("Received midi message: %i %i %i",(int)channel, 
	   (int)midicontrol,(int)midivalue);
    
    // Check the potmeters:
    for (int i=0; i<no_potmeters; i++) 
      if (potmeters[i]->midino == midicontrol) {
	potmeters[i]->slotSetPosition((int)midivalue);
	qDebug("Changed potmeter %s to %i",potmeters[i]->print(), 
	       (int)potmeters[i]->getValue());
	break;
      }
    
    // Check the buttons:
    for (int i=0; i<no_buttons; i++) 
      if (buttons[i]->midino == midicontrol) {
	// Now that we've found a button on the right gate, we try to
	// see if the button is really changed:
	positionType state = down;
	if ((buttons[i]->midimask & midivalue) == 0) state = up;
	if (buttons[i]->position != state) {
	  buttons[i]->slotSetPosition(state);
	  qDebug("Changed button %s to %s", buttons[i]->print(),
		 buttons[i]->printValue());
	  break;
	}
      }
  }
};
