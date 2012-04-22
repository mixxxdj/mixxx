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
class DlgPrefBeats;
class DlgPrefVinyl;
class DlgPrefNoVinyl;
class DlgPrefShoutcast;
class DlgPrefReplayGain;
class PowerMate;
class MidiDeviceManager;
class SkinLoader;
class PlayerManager;
class VinylControlManager;

/**
  *@author Tue & Ken Haste Andersen
  */

class DlgPreferences : public QDialog, public Ui::DlgPreferencesDlg
{
    Q_OBJECT
public:
    DlgPreferences(MixxxApp* mixxx, SkinLoader* pSkinLoader, SoundManager* soundman,
                   PlayerManager* pPlayerManager, MidiDeviceManager* midi,
                   VinylControlManager* pVCManager, ConfigObject<ConfigValue>* config);

    ~DlgPreferences();
    void createIcons();
public slots:
    void slotShow();
    void slotHide();
    void rescanMidi();
    void slotApply();
    void changePage(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void showSoundHardwarePage();
    void slotHighlightDevice(DlgPrefMidiBindings* dialog, bool enabled);
signals:
    void closeDlg();
    void showDlg();
protected:
    bool eventFilter(QObject*, QEvent*);
private:
    void destroyMidiWidgets();
    void setupMidiWidgets();
    int addPageWidget(QWidget* w);
    QList<DlgPrefMidiBindings*> m_wmidiBindingsForDevice;

    DlgPrefSound* m_wsound;
    DlgPrefPlaylist* m_wplaylist;
    DlgPrefNoMidi* m_wNoMidi;
    DlgPrefControls* m_wcontrols;
    DlgPrefEQ* m_weq;
    DlgPrefCrossfader* m_wcrossfader;
    DlgPrefRecord* m_wrecord;
    DlgPrefBpm* m_wbpm;
    DlgPrefBeats* m_wbeats;
    DlgPrefVinyl* m_wvinylcontrol;
    DlgPrefNoVinyl* m_wnovinylcontrol;
    DlgPrefShoutcast* m_wshoutcast;
    DlgPrefReplayGain* m_wreplaygain;

    /*
    QScrollArea* m_sasound;
    QScrollArea* m_saplaylist;
    QScrollArea* m_saNoMidi;
    QScrollArea* m_sacontrols;
    QScrollArea* m_saeq;
    QScrollArea* m_sacrossfader;
    QScrollArea* m_sarecord;
    QScrollArea* m_sabpm;
    QScrollArea* m_savinylcontrol;
    QScrollArea* m_sanovinylcontrol;
    QScrollArea* m_sashoutcast;
    QScrollArea* m_sareplaygain;
	*/

    QTreeWidgetItem* m_pSoundButton;
    QTreeWidgetItem* m_pPlaylistButton;
    QTreeWidgetItem* m_pControlsButton;
    QTreeWidgetItem* m_pEqButton;
    QTreeWidgetItem* m_pCrossfaderButton;
    QTreeWidgetItem* m_pRecordingButton;
    QTreeWidgetItem* m_pBPMdetectButton;
    QTreeWidgetItem* m_pAnalysersButton;
    QTreeWidgetItem* m_pVinylControlButton;
    QTreeWidgetItem* m_pShoutcastButton;
    QTreeWidgetItem* m_pReplayGainButton;
    QTreeWidgetItem* m_pMIDITreeItem;
    QList<QTreeWidgetItem*> m_midiBindingsButtons;

    QSize m_pageSizeHint;

    ConfigObject<ConfigValue>* config;
    MidiDeviceManager* m_pMidiDeviceManager;
};

#endif
