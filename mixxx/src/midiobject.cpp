#include "midiobject.h"
#include "configobject.h"
#include "controlobject.h"
#include "controlpushbutton.h"
#include <algorithm>

// Static member variable definition
ConfigObject<ConfigValueMidi> *MidiObject::config = 0;

/* -------- ------------------------------------------------------
   Purpose: Initialize midi, and start parsing loop
   Input:   None. Automatically selects default midi input sound
            card and device.
   Output:  -
   -------- ------------------------------------------------------ */
MidiObject::MidiObject(ConfigObject<ConfigValueMidi> *c, QApplication *a)
{
    app = a;
    config = c;
    no = 0;
    requestStop = false;
};

/* -------- ------------------------------------------------------
  Purpose: Deallocates midi buffer, and closes device
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
MidiObject::~MidiObject()
{
};

void MidiObject::reopen(QString device)
{
    devClose();
    devOpen(device);
}

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
    //qDebug("Registered midi control %s (%p).", c->print()->ascii(),c);
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

QStringList *MidiObject::getDeviceList()
{
    return &devices;
}

QString *MidiObject::getOpenDevice()
{
    return &openDevice;
}

void MidiObject::stop()
{
    requestStop = true;
}

/* -------- ------------------------------------------------------
   Purpose: Loop for parsing midi events
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
void MidiObject::send(char channel, char midicontrol, char midivalue)
{
    //qDebug("Received midi message: ch %i no %i val %i",(int)channel,(int)midicontrol,(int)midivalue);

    // Check the potmeters:
    for (int i=0; i<no; i++)
    {
        //qDebug("(%i) checking: no %i ch %i",i,(int)controlList[i]->cfgOption->val->midino,(int)controlList[i]->cfgOption->val->midichannel);
        if (controlList[i]->cfgOption->val->midino == midicontrol &
            controlList[i]->cfgOption->val->midichannel == channel)
        {
            // Check for possible bit mask
            int midimask = controlList[i]->cfgOption->val->midimask;

            // Gain app lock
            app->lock();

            if (midimask > 0)
                controlList[i]->slotSetPosition((int)(midimask & midivalue));
            else
                controlList[i]->slotSetPositionMidi((int)midivalue); // 127-midivalue

            // Force GUI update
            app->flush();

            // Release app lock
            app->unlock();

            break;
        }
    }
};

