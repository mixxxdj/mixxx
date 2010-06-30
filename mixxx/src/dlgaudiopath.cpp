/**
 * @file dlgaudiopath.cpp
 * @author Bill Good <bkgood at gmail dot com>
 * @date 20100626
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtCore>
#include "dlgaudiopath.h"
#include "audiopath.h"

DlgAudioPath::DlgAudioPath(QWidget *parent)
    : QDialog(parent) {
    setupUi(this);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    connect(inputRadioButton, SIGNAL(clicked()), this, SLOT(ioChanged()));
    connect(outputRadioButton, SIGNAL(clicked()), this, SLOT(ioChanged()));
    connect(deviceComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(deviceChanged()));
    connect(typeComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(typeChanged()));

    outputRadioButton->click(); // sensible default
}

DlgAudioPath::~DlgAudioPath() {
}

AudioPath DlgAudioPath::getPath() const {
    switch (m_io) {
    case DlgAudioPath::INPUT:
        return AudioReceiver(m_type, m_channelBase, m_channels, m_index);
    case DlgAudioPath::OUTPUT:
        return AudioSource(m_type, m_channelBase, m_channels, m_index);
    }
}
    
void DlgAudioPath::populateDevices() {
    qDebug() << "clearing";
    deviceComboBox->clear();
    qDebug() << "adding none";
    deviceComboBox->addItem("None");
    // fill deviceComboBox with devices accepting i/o respectively
    switch (m_io) {
    case INPUT:
        break;
    case OUTPUT:
        break;
    }
    deviceLabel->setEnabled(true);
    deviceComboBox->setEnabled(true);
    qDebug() << "selecting 0";
    deviceComboBox->setCurrentIndex(0);
}

void DlgAudioPath::populateChannels() {
    // ask the current sound device how many channels it has for the
    // given mode
    qDebug() << "populating channels";
    channelComboBox->clear();
    switch (m_io) {
    case INPUT:
        break;
    case OUTPUT:
        break;
    }
    channelLabel->setEnabled(true);
    channelComboBox->setEnabled(true);
}

void DlgAudioPath::populateTypes() {
    typeComboBox->clear();
    QList<AudioPath::AudioPathType> types;
    switch (m_io) {
    case INPUT:
        types = AudioReceiver::getSupportedTypes();
        break;
    case OUTPUT:
        types = AudioSource::getSupportedTypes();
        break;
    }
    foreach (AudioPath::AudioPathType type, types) {
        typeComboBox->addItem(
            AudioPath::getStringFromType(type), QVariant(type)
        );
    }
    typeComboBox->setCurrentIndex(0);
    typeComboBox->setEnabled(true);
    typeLabel->setEnabled(true);
}

void DlgAudioPath::ioChanged() {
    if (inputRadioButton->isChecked()) {
        qDebug() << "************* input!";
        if (m_io != DlgAudioPath::INPUT) {
            m_io = DlgAudioPath::INPUT;
            populateDevices();
            populateTypes();
        }
    } else if (outputRadioButton->isChecked()) {
        qDebug() << "************* output!";
        if (m_io != DlgAudioPath::OUTPUT) {
            m_io = DlgAudioPath::OUTPUT;
            populateDevices();
            populateTypes();
        }
    }
}

void DlgAudioPath::deviceChanged() {
    populateChannels();
}

void DlgAudioPath::typeChanged() {
    // if type is indexable, enable index
    AudioPath::AudioPathType currentType =
        AudioPath::getTypeFromInt(
            typeComboBox->itemData(typeComboBox->currentIndex()).toInt()
        );
    if (AudioPath::isIndexable(currentType)) {
        indexLabel->setEnabled(true); 
        indexSpinBox->setEnabled(true);
    } else {
        indexLabel->setEnabled(false); 
        indexSpinBox->setEnabled(false);
    }
    indexSpinBox->setValue(1);
}
