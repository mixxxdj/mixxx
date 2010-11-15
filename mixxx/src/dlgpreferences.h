/***************************************************************************
                          dlgpreferences.h  -  description
                             -------------------
    begin                : Sun Jun 30 2002
    copyright            : (C) 2002 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGPREFERENCES_H
#define DLGPREFERENCES_H

 #include <QDialog>

#include <qevent.h>
#include <QtGui>
#include "ui_dlgpreferencesdlg.h"
#include "configobject.h"

class QListWidget;
class QListWidgetItem;
class QStackedWidget;

class MixxxApp;
class PlayerProxy;
class SoundManager;
class DlgPrefSound;
class DlgPrefMidiBindings;
class DlgPrefPlaylist;
class DlgPrefNoMidi;
class DlgPrefControls;
class DlgPrefEQ;
class DlgPrefCrossfader;
class DlgPrefRecord;
class DlgPrefBpm;
class DlgPrefVinyl;
class DlgPrefShoutcast;
class DlgPrefReplayGain;
class PowerMate;
class MidiDeviceManager;
class SkinLoader;

/**
  *@author Tue & Ken Haste Andersen
  */

class DlgPreferences : public QDialog, public Ui::DlgPreferencesDlg
{
    Q_OBJECT
public:
    DlgPreferences(MixxxApp *mixxx, SkinLoader* pSkinLoader, SoundManager *soundman,
                   MidiDeviceManager* midi, ConfigObject<ConfigValue> *config);
    ~DlgPreferences();
    void createIcons();
public slots:
    void slotShow();
    void slotHide();
    void rescanMidi();
    void slotApply();
    void changePage(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void showSoundHardwarePage();
    void slotHighlightDevice(DlgPrefMidiBindings* dialog, bool enabled);
signals:
    void closeDlg();
    void showDlg();
protected:
    bool eventFilter(QObject *, QEvent *);
private:
    void destroyMidiWidgets();
    void setupMidiWidgets();
    DlgPrefSound *wsound;
    QList<DlgPrefMidiBindings*> wmidiBindingsForDevice;
    DlgPrefPlaylist *wplaylist;
    DlgPrefNoMidi *wNoMidi;
    DlgPrefControls *wcontrols;
    DlgPrefEQ *weq;
    DlgPrefCrossfader *wcrossfader;
    DlgPrefRecord *wrecord;
    DlgPrefBpm *wbpm;
    DlgPrefVinyl *wvinylcontrol;
    DlgPrefShoutcast *wshoutcast;
    DlgPrefReplayGain *wreplaygain;

	QTreeWidgetItem* m_pSoundButton;
	QTreeWidgetItem* m_pPlaylistButton;
	QTreeWidgetItem* m_pControlsButton;
	QTreeWidgetItem* m_pEqButton;
	QTreeWidgetItem* m_pCrossfaderButton;
	QTreeWidgetItem* m_pRecordingButton;
	QTreeWidgetItem* m_pBPMdetectButton;
	QTreeWidgetItem* m_pVinylControlButton;
	QTreeWidgetItem* m_pShoutcastButton;
	QTreeWidgetItem* m_pReplayGainButton;
    QTreeWidgetItem* m_pMIDITreeItem;
    QList<QTreeWidgetItem*> m_midiBindingsButtons;

    ConfigObject<ConfigValue> *config;
    MixxxApp *m_pMixxx;
    MidiDeviceManager* m_pMidiDeviceManager;
};

#endif
