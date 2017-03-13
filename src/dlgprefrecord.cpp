/***************************************************************************
                          dlgprefrecord.cpp  -  description
                             -------------------
    begin                : Thu Jun 19 2007
    copyright            : (C) 2007 by John Sully
    email                : jsully@scs.ryerson.ca
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QFileDialog>
#include <QDesktopServices>

#include "dlgprefrecord.h"
#include "recording/defs_recording.h"
#include "controlobject.h"
#include "encoder/encoder.h"
#include "controlobjectthread.h"
#include "util/sandbox.h"

DlgPrefRecord::DlgPrefRecord(QWidget* parent, ConfigObject<ConfigValue>* pConfig)
        : DlgPreferencePage(parent),
          m_pConfig(pConfig),
          m_bConfirmOverwrite(false),
          m_pRadioOgg(NULL),
          m_pRadioMp3(NULL),
          m_pRadioAiff(NULL),
          m_pRadioFlac(NULL),
          m_pRadioWav(NULL) {
    setupUi(this);

    // See RECORD_* #defines in defs_recording.h
    m_pRecordControl = new ControlObjectThread(
            RECORDING_PREF_KEY, "status");

    m_pRadioOgg = new QRadioButton("Ogg Vorbis");
    m_pRadioMp3 = new QRadioButton(ENCODING_MP3);

    // Setting recordings path.
    QString recordingsPath = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Directory"));
    if (recordingsPath == "") {
        // Initialize recordings path in config to old default path.
        // Do it here so we show current value in UI correctly.
        QString musicDir = QDesktopServices::storageLocation(QDesktopServices::MusicLocation);
        QDir recordDir(musicDir + "/Mixxx/Recordings");
        recordingsPath = recordDir.absolutePath();
    }
    LineEditRecordings->setText(recordingsPath);

    connect(PushButtonBrowseRecordings, SIGNAL(clicked()),
            this, SLOT(slotBrowseRecordingsDir()));
    connect(LineEditRecordings, SIGNAL(returnPressed()),
            this, SLOT(slotApply()));

    connect(m_pRadioOgg, SIGNAL(clicked()),
            this, SLOT(slotApply()));
    connect(m_pRadioMp3, SIGNAL(clicked()),
            this, SLOT(slotApply()));
    horizontalLayout->addWidget(m_pRadioOgg);
    horizontalLayout->addWidget(m_pRadioMp3);

    // AIFF and WAVE are supported by default.
    m_pRadioWav = new QRadioButton(ENCODING_WAVE);
    connect(m_pRadioWav, SIGNAL(clicked()), this, SLOT(slotApply()));
    horizontalLayout->addWidget(m_pRadioWav);

    m_pRadioAiff = new QRadioButton(ENCODING_AIFF);
    connect(m_pRadioAiff, SIGNAL(clicked()), this, SLOT(slotApply()));
    horizontalLayout->addWidget(m_pRadioAiff);

#ifdef SF_FORMAT_FLAC
    m_pRadioFlac = new QRadioButton(ENCODING_FLAC);
    connect(m_pRadioFlac, SIGNAL(clicked()), this, SLOT(slotApply()));
    horizontalLayout->addWidget(m_pRadioFlac);
#endif

    // Read config and check radio button.
    QString format = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Encoding"));
    if (format == ENCODING_WAVE) {
        m_pRadioWav->setChecked(true);
    } else if (format == ENCODING_OGG) {
        m_pRadioOgg->setChecked(true);
    } else if (format == ENCODING_MP3) {
        m_pRadioMp3->setChecked(true);
    } else if (format == ENCODING_AIFF) {
        m_pRadioAiff->setChecked(true);
#ifdef SF_FORMAT_FLAC
    } else if (format == ENCODING_FLAC) {
        m_pRadioFlac->setChecked(true);
#endif
    } else {
        // Invalid, so set default and save.
        // If no config was available, set to WAVE as default.
        m_pRadioWav->setChecked(true);
        m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(ENCODING_WAVE));
    }

    loadMetaData();

    connect(SliderQuality, SIGNAL(valueChanged(int)),
            this, SLOT(slotSliderQuality()));
    connect(SliderQuality, SIGNAL(sliderMoved(int)),
            this, SLOT(slotSliderQuality()));
    connect(SliderQuality, SIGNAL(sliderReleased()),
            this, SLOT(slotSliderQuality()));
    connect(CheckBoxRecordCueFile, SIGNAL(stateChanged(int)),
            this, SLOT(slotEnableCueFile(int)));
    connect(comboBoxSplitting, SIGNAL(activated(int)),
            this, SLOT(slotChangeSplitSize()));

    slotApply();
    // Make sure a corrupt config file won't cause us to record constantly.
    m_pRecordControl->slotSet(RECORD_OFF);

    comboBoxSplitting->addItem(SPLIT_650MB);
    comboBoxSplitting->addItem(SPLIT_700MB);
    comboBoxSplitting->addItem(SPLIT_1024MB);
    comboBoxSplitting->addItem(SPLIT_2048MB);
    comboBoxSplitting->addItem(SPLIT_4096MB);

    QString fileSizeStr = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "FileSize"));
    int index = comboBoxSplitting->findText(fileSizeStr);
    if (index > 0) {
        // Set file split size
        comboBoxSplitting->setCurrentIndex(index);
    }
    // Otherwise 650 MB will be default file split size.

    // Read CUEfile info
    CheckBoxRecordCueFile->setChecked(
            (bool) m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "CueEnabled")).toInt());

}

void DlgPrefRecord::slotSliderQuality() {
    updateTextQuality();

    if (m_pRadioOgg && m_pRadioOgg->isChecked()) {
        m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "OGG_Quality"), ConfigValue(SliderQuality->value()));
    } else if (m_pRadioMp3 && m_pRadioMp3->isChecked()) {
        m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "MP3_Quality"), ConfigValue(SliderQuality->value()));
    }
}

int DlgPrefRecord::getSliderQualityVal() {
    // We always use the bitrate to denote the quality since it is more common to the users.
    return Encoder::convertToBitrate(SliderQuality->value());
}

void DlgPrefRecord::updateTextQuality() {
    int quality = getSliderQualityVal();
    //QString encodingType = comboBoxEncoding->currentText();

    TextQuality->setText(QString(QString::number(quality) + tr("kbps")));
}

void DlgPrefRecord::slotEncoding() {
    // set defaults
    groupBoxQuality->setEnabled(true);
    //m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(comboBoxEncoding->currentText()));

    if (m_pRadioWav && m_pRadioWav->isChecked()) {
        m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(ENCODING_WAVE));
        groupBoxQuality->setEnabled(false);
    } else if (m_pRadioFlac && m_pRadioFlac->isChecked()) {
        m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(ENCODING_FLAC));
        groupBoxQuality->setEnabled(false);
    } else if (m_pRadioAiff && m_pRadioAiff->isChecked()) {
        m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(ENCODING_AIFF));
        groupBoxQuality->setEnabled(false);
    } else if (m_pRadioOgg && m_pRadioOgg->isChecked()) {
        int value = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "OGG_Quality")).toInt();
        // If value == 0 then a default value of 128kbps is proposed.
        if (!value)
            value = 6; // 128kbps

        SliderQuality->setValue(value);
        m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(ENCODING_OGG));
    } else if (m_pRadioMp3 && m_pRadioMp3->isChecked()) {
        int value = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "MP3_Quality")).toInt();
        // If value == 0 then a default value of 128kbps is proposed.
        if (!value) {
            value = 6;  // 128kbps
        }

        SliderQuality->setValue(value);
        m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(ENCODING_MP3));
    } else {
        qDebug() << "Invalid recording encoding type in" << __FILE__ << "on line:" << __LINE__;
    }
}

void DlgPrefRecord::setMetaData() {
    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Title"), ConfigValue(LineEditTitle->text()));
    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Author"), ConfigValue(LineEditAuthor->text()));
    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Album"), ConfigValue(LineEditAlbum->text()));
}

void DlgPrefRecord::loadMetaData() {
    LineEditTitle->setText(m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Title")));
    LineEditAuthor->setText(m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Author")));
    LineEditAlbum->setText(m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Album")));
}

DlgPrefRecord::~DlgPrefRecord() {
   delete m_pRecordControl;
}

void DlgPrefRecord::slotRecordPathChange() {
    m_bConfirmOverwrite = false;
    slotApply();
}

void DlgPrefRecord::slotResetToDefaults() {
    m_pRadioWav->setChecked(true);
    CheckBoxRecordCueFile->setChecked(false);
    // 650MB splitting is the default
    comboBoxSplitting->setCurrentIndex(0);

    LineEditTitle->setText("");
    LineEditAlbum->setText("");
    LineEditAuthor->setText("");

    // 6 corresponds to 128kbps (only used by MP3 and Ogg though)
    SliderQuality->setValue(6);
}

// This function updates/refreshes the contents of this dialog.
void DlgPrefRecord::slotUpdate() {

    QString recordingsPath = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Directory"));
    LineEditRecordings->setText(recordingsPath);

    if (m_pRadioWav && m_pRadioWav->isChecked()) {
        m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(ENCODING_WAVE));
    } else if (m_pRadioAiff && m_pRadioAiff->isChecked()) {
        m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(ENCODING_AIFF));
    } else if (m_pRadioFlac && m_pRadioFlac->isChecked()) {
        m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(ENCODING_FLAC));
    } else if (m_pRadioOgg && m_pRadioOgg->isChecked()) {
        m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(ENCODING_OGG));
    } else if (m_pRadioMp3 && m_pRadioMp3->isChecked()) {
       m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(ENCODING_MP3));
    }
    loadMetaData();
}

void DlgPrefRecord::slotBrowseRecordingsDir() {
    QString fd = QFileDialog::getExistingDirectory(
            this, tr("Choose recordings directory"),
            m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY,
                                                "Directory")));

    if (fd != "") {
        // The user has picked a new directory via a file dialog. This means the
        // system sandboxer (if we are sandboxed) has granted us permission to
        // this folder. Create a security bookmark while we have permission so
        // that we can access the folder on future runs. We need to canonicalize
        // the path so we first wrap the directory string with a QDir.
        QDir directory(fd);
        Sandbox::createSecurityToken(directory);
        LineEditRecordings->setText(fd);
    }
}

void DlgPrefRecord::slotApply() {
    setRecordingFolder();
    setMetaData();
    slotEncoding();
}

void DlgPrefRecord::setRecordingFolder() {
    if (LineEditRecordings->text() == "") {
        qDebug() << "Recordings path was empty in dialog";
        return;
    }
    if (LineEditRecordings->text() != m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Directory"))) {
        qDebug() << "Saved recordings path" << LineEditRecordings->text();
        m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Directory"), LineEditRecordings->text());
    }
}

void DlgPrefRecord::slotEnableCueFile(int enabled) {
    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "CueEnabled"), ConfigValue(enabled != Qt::Unchecked));
}

void DlgPrefRecord::slotChangeSplitSize() {
        m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "FileSize"),
                    ConfigValue(comboBoxSplitting->currentText()));

}
