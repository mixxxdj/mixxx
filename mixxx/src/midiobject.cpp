#include "midiobject.h"
#include "configobject.h"
#include "controlobject.h"
#include "controlpushbutton.h"
#include <qevent.h>
#include <algorithm>
#include <qdir.h>

// Static member variable definition
ConfigObject<ConfigValueMidi> *MidiObject::config = 0;

/* -------- ------------------------------------------------------
   Purpose: Initialize midi, and start parsing loop
   Input:   None. Automatically selects default midi input sound
            card and device.
   Output:  -
   -------- ------------------------------------------------------ */
MidiObject::MidiObject(ConfigObject<ConfigValueMidi> *c, QApplication *a, QWidget *m, QString)
{
    app = a;
    mixxx = m;

    config = c;
    no = 0;
    requestStop = false;
}

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
//    qDebug("Registered midi control %s (%p).", c->print()->ascii(),c);
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
        qWarning("MidiObject: Control which is requested for removal does not exist.");
}

QStringList *MidiObject::getDeviceList()
{
    return &devices;
}

QStringList *MidiObject::getConfigList(QString path)
{
    // Make sure list is empty
    configs.clear();
    
    // Get list of available midi configurations
    QDir dir(path);
    dir.setFilter(QDir::Files);
    dir.setNameFilter("*.midi.cfg *.MIDI.CFG");
    const QFileInfoList *list = dir.entryInfoList();
    if (list!=0)
    {
        QFileInfoListIterator it(*list);        // create list iterator
        QFileInfo *fi;                          // pointer for traversing
        while ((fi=it.current()))
        {
qDebug("..");
            configs.append(fi->fileName());
            ++it;   // goto next list element
        }
    }
    return &configs;
}

QString *MidiObject::getOpenDevice()
{
    return &openDevice;
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
        //qDebug("(%i) checking: no %i ch %i [%p]", i, controlList[i]->cfgOption->val->midino, 
        //               controlList[i]->cfgOption->val->midichannel);
        //cout << "value: " << controlList[i]->cfgOption->val->value << "\n";
        if ((controlList[i]->cfgOption->val->midino == midicontrol) &
            (controlList[i]->cfgOption->val->midichannel == channel))
        {
            // Check for possible bit mask
            int midimask = controlList[i]->cfgOption->val->midimask;

            if (midimask > 0)
                controlList[i]->slotSetPosition((int)(midimask & midivalue));
            else
                controlList[i]->slotSetPositionMidi((int)midivalue); // 127-midivalue

            // Send User event, to force screen update
            postEvent(mixxx,new QEvent(QEvent::User));

            break;
        }
    }
};

void MidiObject::stop()
{
    requestStop = true;
}

void abortRead(int)
{
    // Reinstall default handler
    signal(SIGINT,SIG_DFL);

    // End thread execution
    QThread::exit();
}

