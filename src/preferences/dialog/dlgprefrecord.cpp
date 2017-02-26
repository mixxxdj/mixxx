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

#include "preferences/dialog/dlgprefrecord.h"
#include "recording/defs_recording.h"
#include "control/controlobject.h"
#include "encoder/encoder.h"
#include "control/controlproxy.h"
#include "util/sandbox.h"

DlgPrefRecord::DlgPrefRecord(QWidget* parent, UserSettingsPointer pConfig)
        : DlgPreferencePage(parent),
          m_selFormat("","",false),
          m_pConfig(pConfig) {
    setupUi(this);

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

    // Setting Encoder
    bool found = false;
    QString prefformat = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Encoding"));
    foreach( Encoder::Format format, EncoderFactory::getFactory().getFormats()) {
        QRadioButton* button = new QRadioButton(format.label, this);
        button->setObjectName(format.internalName);
        connect(button, SIGNAL(clicked()), this, SLOT(slotFormatChanged()));
        EncoderLayout->addWidget(button);

        if (prefformat == format.internalName) {
            m_selFormat = format;
            button->setChecked(true);
            found=true;
        }
        m_formatButtonss.append(button);
    }
    if (!found) {
        // If no format was available, set to WAVE as default.
        if (!prefformat.isEmpty()) {
            qWarning() << prefformat <<" format was set in the configuration, but it is not recognized!";
        }
        m_selFormat = EncoderFactory::getFactory().getFormats().first();
        m_formatButtonss.first()->setChecked(true);
        m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"),  ConfigValue(m_selFormat.internalName));
    }

    setupEncoderUI(m_selFormat);

    // Setting Metadata
    loadMetaData();

    // Setting miscellaneous
    CheckBoxRecordCueFile->setChecked(
            (bool) m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "CueEnabled")).toInt());
    connect(CheckBoxRecordCueFile, SIGNAL(stateChanged(int)),
            this, SLOT(slotEnableCueFile(int)));

    // Setting split
    comboBoxSplitting->addItem(SPLIT_650MB);
    comboBoxSplitting->addItem(SPLIT_700MB);
    comboBoxSplitting->addItem(SPLIT_1024MB);
    comboBoxSplitting->addItem(SPLIT_2048MB);
    comboBoxSplitting->addItem(SPLIT_4096MB);
    comboBoxSplitting->addItem(SPLIT_60MIN);
    comboBoxSplitting->addItem(SPLIT_74MIN);
    comboBoxSplitting->addItem(SPLIT_80MIN);
    comboBoxSplitting->addItem(SPLIT_120MIN);

    QString fileSizeStr = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "FileSize"));
    int index = comboBoxSplitting->findText(fileSizeStr);
    if (index >= 0) {
        // Set file split size
        comboBoxSplitting->setCurrentIndex(index);
    }
    else {
        //Use max RIFF size (4GB) as default index, since usually people don't want to split.
        comboBoxSplitting->setCurrentIndex(4);
    }
    connect(comboBoxSplitting, SIGNAL(activated(int)),
            this, SLOT(slotChangeSplitSize()));

    // Do the one-time connection of signals here.
    connect(SliderQuality, SIGNAL(valueChanged(int)),
            this, SLOT(slotSliderQuality()));
    connect(SliderQuality, SIGNAL(sliderMoved(int)),
            this, SLOT(slotSliderQuality()));
    connect(SliderQuality, SIGNAL(sliderReleased()),
            this, SLOT(slotSliderQuality()));
    connect(SliderCompression, SIGNAL(valueChanged(int)),
            this, SLOT(slotSliderCompression()));
    connect(SliderCompression, SIGNAL(sliderMoved(int)),
            this, SLOT(slotSliderCompression()));
    connect(SliderCompression, SIGNAL(sliderReleased()),
            this, SLOT(slotSliderCompression()));
}

DlgPrefRecord::~DlgPrefRecord() {
    // Note: I don't disconnect signals, since that's supposedly done automatically
    // when the object is deleted
    foreach(QRadioButton* button, m_formatButtonss) {
        // TODO: Not sure if this is necessary or correct, or I should simply "delete button;"
        emit(button->deleteLater());
    }
}

void DlgPrefRecord::slotApply() {
    saveRecordingFolder();
    saveMetaData();
    saveEncoding();
    saveUseCueFile();
    saveSplitSize();
}

// This function updates/refreshes the contents of this dialog.
void DlgPrefRecord::slotUpdate() {

    QString recordingsPath = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Directory"));
    LineEditRecordings->setText(recordingsPath);

   m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), 
        ConfigValue(m_selFormat.internalName));

//    setupEncoderUI(m_selFormat);
 
    loadMetaData();
}

void DlgPrefRecord::slotResetToDefaults() {
    m_formatButtonss.first()->setChecked(true);
    setupEncoderUI(EncoderFactory::getFactory().getFormatFor(m_formatButtonss.first()->objectName()));
    // TODO: There really should be a defaultSettings() option 
    // in the EncoderSettings interface
    static_cast<QAbstractButton*>(m_optionWidgets.first())->setChecked(true);

    LineEditTitle->setText("");
    LineEditAlbum->setText("");
    LineEditAuthor->setText("");

    // 4GB splitting is the default
    comboBoxSplitting->setCurrentIndex(4);
    CheckBoxRecordCueFile->setChecked(false);

}


void DlgPrefRecord::slotBrowseRecordingsDir() {
    QString fd = QFileDialog::getExistingDirectory(
            this, tr("Choose recordings directory"),
            m_pConfig->getValueString(
                    ConfigKey(RECORDING_PREF_KEY,"Directory")));

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

void DlgPrefRecord::slotFormatChanged()
{
    QObject *senderObj = sender();
    m_selFormat = EncoderFactory::getFactory().getFormatFor(senderObj->objectName());
    setupEncoderUI(m_selFormat);
}

void DlgPrefRecord::setupEncoderUI(Encoder::Format selformat)
{
    EncoderSettingsPointer settings = EncoderFactory::getFactory().getEncoderSettings(selformat, m_pConfig);
    if (settings->usesQualitySlider()) {
        groupBoxQuality->setVisible(true);
        SliderQuality->setMinimum(0);
        SliderQuality->setMaximum(settings->getQualityValues().size()-1);
        SliderQuality->setValue(settings->getQualityIndex());
    } else {
        groupBoxQuality->setVisible(false);
    }
    if (settings->usesCompressionSlider()) {
        groupBoxCompression->setVisible(true);
        SliderCompression->setMinimum(0);
        SliderCompression->setMaximum(settings->getCompressionValues().size()-1);
        SliderCompression->setValue(settings->getCompression());
    } else {
        groupBoxCompression->setVisible(false);
    }

    foreach(QWidget* widget, m_optionWidgets) {
        groupBoxOptions->layout()->removeWidget(widget);
        disconnect(widget, SIGNAL(clicked()), this, SLOT(slotGroupChanged()));
        emit(widget->deleteLater());
    }
    m_optionWidgets.clear();
    if (settings->usesOptionGroups()) {
        groupBoxOptions->setVisible(true);
        // TODO: Right now i am supporting just one optiongroup.
        // The concept is already there for multiple groups
        EncoderSettings::OptionsGroup group = settings->getOptionGroups().first();
        groupBoxOptions->setTitle(group.groupName);
        groupBoxOptions->setObjectName(group.groupCode);
        int controlIdx = settings->getSelectedOption(group.groupCode);
        foreach(QString name, group.controlNames) {
            QAbstractButton* widget;
            if (group.controlNames.size() == 1) {
                QCheckBox* button = new QCheckBox(name, this);
                widget = button;
            } else {
                QRadioButton* button = new QRadioButton(name, this);
                widget = button;
            }
            connect(widget, SIGNAL(clicked()), this, SLOT(slotGroupChanged()));
            widget->setObjectName(group.groupCode);
            groupBoxOptions->layout()->addWidget(widget);
            m_optionWidgets.append(widget);
            if (controlIdx == 0 ) {
                widget->setChecked(true);
            }
            controlIdx--;
        }
    } else {
        groupBoxOptions->setVisible(false);
    }
}

void DlgPrefRecord::slotSliderQuality() {
    updateTextQuality();
    // Settings are only stored when doing an apply so that "cancel" can actually cancel.
}

void DlgPrefRecord::updateTextQuality() {
    EncoderSettingsPointer settings = EncoderFactory::getFactory().getEncoderSettings(m_selFormat, m_pConfig);
    int quality = settings->getQualityValues().at(SliderQuality->value());
    TextQuality->setText(QString(QString::number(quality) + tr("kbps")));
}

void DlgPrefRecord::slotSliderCompression() {
    updateTextCompression();
    // Settings are only stored when doing an apply so that "cancel" can actually cancel.
}
void DlgPrefRecord::updateTextCompression() {
    EncoderSettingsPointer settings = EncoderFactory::getFactory().getEncoderSettings(m_selFormat, m_pConfig);
    int quality = settings->getCompressionValues().at(SliderCompression->value());
    TextCompression->setText(QString::number(quality));
}

void DlgPrefRecord::slotGroupChanged()
{
    // On complex scenarios, one could want to enable or disable some controls when changing
    // these, but we don't have these needs now.
}

void DlgPrefRecord::loadMetaData() {
    LineEditTitle->setText(m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Title")));
    LineEditAuthor->setText(m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Author")));
    LineEditAlbum->setText(m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Album")));
}


void DlgPrefRecord::saveRecordingFolder()
{
    if (LineEditRecordings->text() == "") {
        qDebug() << "Recordings path was empty in dialog";
        return;
    }
    if (LineEditRecordings->text() != m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Directory"))) {
        qDebug() << "Saved recordings path" << LineEditRecordings->text();
        m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Directory"), LineEditRecordings->text());
    }
}
void DlgPrefRecord::saveMetaData()
{
    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Title"), ConfigValue(LineEditTitle->text()));
    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Author"), ConfigValue(LineEditAuthor->text()));
    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Album"), ConfigValue(LineEditAlbum->text()));
}
void DlgPrefRecord::saveEncoding()
{
    EncoderSettingsPointer settings = EncoderFactory::getFactory().getEncoderSettings(m_selFormat, m_pConfig);

    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"),
        ConfigValue(m_selFormat.internalName));

    if (settings->usesQualitySlider()) {
        settings->setQualityByIndex(SliderQuality->value());
    }
    if (settings->usesCompressionSlider()) {
        QList<int> comps = settings->getCompressionValues();
        settings->setCompression(comps.at(SliderCompression->value()));
    }
    if (settings->usesOptionGroups()) {
        // TODO: Right now i am supporting just one optiongroup.
        // The concept is already there for multiple groups
        EncoderSettings::OptionsGroup group = settings->getOptionGroups().first();
        int i=0;
        foreach(QWidget* widget, m_optionWidgets) {
            if (widget->objectName() == group.groupCode) {
                if (static_cast<QAbstractButton*>(widget)->isChecked() != Qt::Unchecked) {
                    settings->setGroupOption(group.groupCode, i);
                    break;
                }
                i++;
            }
        }
    }
}
void DlgPrefRecord::saveUseCueFile()
{
    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "CueEnabled"),
        ConfigValue(CheckBoxRecordCueFile->isChecked()));
}
void DlgPrefRecord::saveSplitSize()
{
    m_pConfig->set(ConfigKey(RECORDING_PREF_KEY, "FileSize"),
                ConfigValue(comboBoxSplitting->currentText()));
}
