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
#include "audiopathmodel.h"

class QWidget;
class SoundManager;
class ControlObject;
class AudioPathModel;

class DlgPrefNewSound : public QWidget, public Ui::DlgPrefNewSoundDlg  {
    Q_OBJECT;
public:
    DlgPrefNewSound(QWidget *parent, SoundManager *soundManager,
            ConfigObject<ConfigValue> *config);
    ~DlgPrefNewSound();
signals:
public slots:
    void slotUpdate();
    void slotApply();
private slots:
    void slotUpdateApi();
    void addClicked();
private:
    void setupDefaultPaths();
    SoundManager *m_pSoundManager;
    ConfigObject<ConfigValue> *m_pConfig;
    AudioPathModel m_model;
};

#endif
