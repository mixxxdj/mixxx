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

};

/* -------- ------------------------------------------------------
  Purpose: Deallocates midi buffer, and closes device
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
MidiObject::~MidiObject() {
  // Close device
  Pm_Close(midi);

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
	std::vector<ControlPushButton*>::iterator iter = 
		std::find(buttons.begin(), buttons.end(), button);
  if (iter != buttons.end())
    buttons.erase(iter);
  else
    qWarning("Pushbutton which is requested for removal in MidiObject does not exist.");
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
	std::vector<ControlPotmeter*>::iterator iter = 
		std::find(potmeters.begin(), potmeters.end(), potmeter);
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
  PmError err;
  char channel, midicontrol, midivalue;
  while(stop == 0) {
    err = Pm_Poll(midi);
    if (err == TRUE) {
          if (Pm_Read(midi, buffer, 1) > 0) {
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

     qDebug("Received midi message: %i %i %i",(int)channel, 
             (int)midicontrol,(int)midivalue);
    
    // Check the potmeters:
    for (int i=0; i<no_potmeters; i++) 
      if (potmeters[i]->midino == midicontrol) {
	  //potmeters[i]->slotSetPosition((int)midivalue);
	potmeters[i]->midiEvent(127-(int)midivalue);
	//qDebug("Changed potmeter %s to %i",potmeters[i]->print(), 
	//       (int)potmeters[i]->getValue());
	break;
      }
    
    // Check the buttons:
    for (i=0; i<no_buttons; i++) 
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
