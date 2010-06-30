/**
 * @file dlgprefnewsound.cpp
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

#include "dlgprefnewsound.h"
#include "dlgaudiopath.h"
#include "soundmanager.h"
#include "sounddevice.h"

DlgPrefNewSound::DlgPrefNewSound(QWidget *parent, SoundManager *soundManager,
        ConfigObject<ConfigValue> *config)
    : QWidget(parent)
    , m_pSoundManager(soundManager)
    , m_pConfig(config)
    , m_model(this, config) {
    setupUi(this);
    m_model.setDevices(soundManager->getDeviceList(MIXXX_PORTAUDIO_ALSA_STRING, true, true));
    audioPathView->setModel(&m_model);
    connect(addButton, SIGNAL(clicked()), this, SLOT(addClicked()));
    m_model.insertRows(0,1);
    m_model.insertRows(1,1);
}

DlgPrefNewSound::~DlgPrefNewSound() {

}

void DlgPrefNewSound::slotUpdate() {

}

void DlgPrefNewSound::slotApply() {

}

void DlgPrefNewSound::slotUpdateApi() {
    // apply defaults
}

void DlgPrefNewSound::addClicked() {
    DlgAudioPath dlg(this);
    int result(dlg.exec());
    if (result == QDialog::Accepted) {
        // look at the diag, insert what we got into the model
    }
}
