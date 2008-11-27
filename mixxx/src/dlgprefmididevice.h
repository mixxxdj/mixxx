/***************************************************************************
                          dlgprefmididevice.h  -  description
                             -------------------
    begin                : Sat Jun 21 2008
    copyright            : (C) 2008 by Tom Care
    email                : psyc0de@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef DLGPREFMIDIDEVICE_H_
#define DLGPREFMIDIDEVICE_H_

#include <QtGui>
#include "ui_dlgprefmididevicedlg.h"
#include "configobject.h"

class MidiObject;

class DlgPrefMidiDevice : public QWidget, public Ui::DlgPrefMidiDeviceDlg  {
    Q_OBJECT
public:
    DlgPrefMidiDevice(QWidget *parent, MidiObject* midi, ConfigObject<ConfigValue> *pConfig);
    ~DlgPrefMidiDevice();

public slots:
    void slotUpdate();
    void slotApply();
    void slotGetInfo(QListWidgetItem * current, QListWidgetItem * previous);
    void slotUpdateEnabled();
    void slotDebug(ConfigValueMidi *event, QString device);

private:
	void saveSettings(QString device, QString Rx, QString Tx); // Save current settings to the configobject
	void readSettings(); // Read in settings from the configobject
	bool deviceEnabled(); // Is the currently selected device enabled?
	void presetAvailable(QString deviceName); // Is there a preset available
	void setupDevices();
    MidiObject *m_pMidi;
    ConfigObject<ConfigValue> *m_pDeviceSettings;
    ConfigObject<ConfigValue> *m_pConfig;
    ConfigObject<ConfigValueMidi> *m_pMidiConfig;
};

#endif /*DLGPREFMIDIDEVICE_H_*/
