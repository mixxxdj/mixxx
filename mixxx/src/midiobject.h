/***************************************************************************
                          midiobject.h  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MIDIOBJECT_H
#define MIDIOBJECT_H

#include <stdio.h>
#include <stdlib.h>

#include <vector>
// #include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <qthread.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qstring.h>
#include <qapplication.h>
#include <qwidget.h>
#include "defs.h"
#include "configobject.h"

class ControlObject;

class MidiObject : public QThread {
public:
    MidiObject(ConfigObject<ConfigValueMidi> *c, QApplication *app, ControlObject *_control, QString device);
    ~MidiObject();
    void reopen(QString device);
    virtual void devOpen(QString) = 0;
    virtual void devClose() = 0;
    void add(ControlObject* c);
    void remove(ControlObject* c);
    /** Returns a list of available devices */
    QStringList *getDeviceList();
    /** Returns the name of the current open device */
    QString *getOpenDevice();
    /** Returns a list of available configurations. Takes as input the directory path
      * containing the configuration files */
    QStringList *getConfigList(QString path);
protected:
    void run() {};
    void stop();
    void send(char channel, char midicontrol, char midivalue);

    bool requestStop;

    static ConfigObject<ConfigValueMidi> *config;
    int                           fd, count, size, no;
    std::vector<ControlObject*>   controlList;

    /** List of available midi devices */
    QStringList devices;
    /** Name of current open device */
    QString openDevice;
    /** List of available midi configurations. Initialized upon call to getConfigList() */
    QStringList configs;

    QApplication *app;
    ControlObject *control;
};

void abortRead(int);

#endif


