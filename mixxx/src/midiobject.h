#ifndef MIDIOBJECT_H
#define MIDIOBJECT_H

#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <qthread.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qstring.h>
#include <qapplication.h>
#include "defs.h"
#include "configobject.h"

class ControlObject;

class MidiObject : public QThread {
public:
    MidiObject(ConfigObject *c, QApplication *app);
    ~MidiObject();
    void reopen(QString device);
    void add(ControlObject* c);
    void remove(ControlObject* c);

    /** Returns a list of available devices */
    QStringList *getDeviceList();

    /** Returns the name of the current open device */
    QString *getOpenDevice();
protected:
    virtual void devOpen(QString device) = 0;
    virtual void devClose() = 0;
    virtual void run() = 0;
    void stop();
    void send(char channel, char midicontrol, char midivalue);

    bool requestStop;
    static ConfigObject           *config;
    int                           fd, count, size, no;
    std::vector<ControlObject*>   controlList;

    /** List of available midi devices */
    QStringList devices;

    /** Name of current open device */
    QString openDevice;

    QApplication *app;
};

#endif


