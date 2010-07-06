/**
 * @file dlgprefnewsound.h
 * @author Bill Good <bkgood at gmail dot com>
 * @date 20100625
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGPREFNEWSOUND_H
#define DLGPREFNEWSOUND_H

#include <QtCore>
#include "ui_dlgprefnewsounddlg.h"
#include "configobject.h"

class SoundManager;
class ControlObject;
class SoundDevice;

const unsigned int NUM_DECKS = 2; // this is temporary... eventually this shoud come from
                                  // soundmanager or something

class DlgPrefNewSound : public QWidget, public Ui::DlgPrefNewSoundDlg  {
    Q_OBJECT;
public:
    DlgPrefNewSound(QWidget *parent, SoundManager *soundManager,
            ConfigObject<ConfigValue> *config);
    ~DlgPrefNewSound();
signals:
    void refreshOutputDevices(QList<SoundDevice*> &devices);
    void refreshInputDevices(QList<SoundDevice*> &devices);
public slots:
    void slotUpdate(); // called on show
    void slotApply();  // called on ok button
private:
    void initializePaths();
    SoundManager *m_pSoundManager;
    ConfigObject<ConfigValue> *m_pConfig;
    QList<SoundDevice*> m_inputDevices;
    QList<SoundDevice*> m_outputDevices;
    bool m_settingsModified;
    QString m_api;
private slots:
    void apiChanged(int index);
    void updateLatencies(int sampleRateIndex);
    void settingChanged();
};

#endif
