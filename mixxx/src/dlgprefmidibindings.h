/***************************************************************************
                          dlgprefmidibindings.h  -  description
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
#ifndef DLGPREFMIDIBINDINGS_H_
#define DLGPREFMIDIBINDINGS_H_

#include <QtGui>
#include "ui_dlgprefmidibindingsdlg.h"
#include "dlgmidilearning.h"
#include "configobject.h"

//Forward declarations
class MidiChannelDelegate;
class MidiStatusDelegate;
class MidiNoDelegate;
class MidiOptionDelegate;
class ControlGroupDelegate;
class ControlValueDelegate;
class MidiDevice;
class MidiDeviceManager;

class DlgPrefMidiBindings : public QWidget, public Ui::DlgPrefMidiBindingsDlg  {
    Q_OBJECT
public:
    DlgPrefMidiBindings(QWidget *parent, MidiDevice* midiDevice,
                        MidiDeviceManager* midiDeviceManager,
    					ConfigObject<ConfigValue> *pConfig);
    ~DlgPrefMidiBindings();


public slots:
    void slotUpdate();
    void slotApply();
    void slotShowMidiLearnDialog();
    void slotLoadMidiMapping(const QString &name);
    void slotExportXML();
    void slotEnableDevice();
    void slotDisableDevice();

    //Input bindings
    void slotClearAllInputBindings();
    void slotRemoveInputBinding();
    void slotAddInputBinding();

    //Output bindings
    void slotAddOutputBinding();
    void slotClearAllOutputBindings();
    void slotRemoveOutputBinding();

private:
    void setRowBackground(int row, QColor color);
    void savePreset(QString path);
    void enumeratePresets();
    void enumerateOutputDevices();
    
    int currentGroupRow;
    MidiChannelDelegate* m_pMidiChannelDelegate;
    MidiStatusDelegate* m_pMidiStatusDelegate;
    MidiNoDelegate* m_pMidiNoDelegate;
    MidiOptionDelegate* m_pMidiOptionDelegate;
    ControlGroupDelegate* m_pControlGroupDelegate;
    ControlValueDelegate* m_pControlValueDelegate;
    QAction* m_deleteMIDIInputRowAction; /** Used for setting up the shortcut for delete button */
    ConfigObject<ConfigValue> *m_pConfig;
    MidiDevice* m_pMidiDevice;
    MidiDeviceManager* m_pMidiDeviceManager;
    DlgMidiLearning* m_pDlgMidiLearning;
};

#endif /*DLGPREFMIDIBINDINGS_H_*/
