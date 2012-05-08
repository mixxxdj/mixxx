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

#include <QtCore>
#include <QtGui>
#include "dlgprefrecord.h"
#include "recording/defs_recording.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "recording/encoder.h"


DlgPrefRecord::DlgPrefRecord(QWidget * parent, ConfigObject<ConfigValue> * _config)
        : QWidget(parent) {
    config = _config;
    confirmOverwrite = false;
    radioFlac = 0;
    radioMp3 = 0;
    radioOgg = 0;
    radioAiff= 0;
    radioWav = 0;


    setupUi(this);

    recordControl = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Master]", "Record"))); //See RECORD_* #defines in defs_recording.h



#ifdef __SHOUTCAST__
    radioOgg = new QRadioButton("Ogg Vorbis");
    radioMp3 = new QRadioButton(ENCODING_MP3);

    // Setting recordings path
    QString recordingsPath = config->getValueString(ConfigKey("[Recording]","Directory"));
    if (recordingsPath == "") {
        // Initialize recordings path in config to old default path.
        // Do it here so we show current value in UI correctly.
        QDir musicDir(config->getValueString(ConfigKey("[Playlist]","Directory")));
        QDir recordDir(musicDir.absolutePath() + "/Mixxx/Recordings");
        recordingsPath = recordDir.absolutePath();
    }
    LineEditRecordings->setText(recordingsPath);

    connect(PushButtonBrowseRecordings, SIGNAL(clicked()), this, SLOT(slotBrowseRecordingsDir()));
    connect(LineEditRecordings, SIGNAL(returnPressed()), this, SLOT(slotApply()));

    connect(radioOgg,SIGNAL(clicked()),
            this, SLOT(slotApply()));
    connect(radioMp3, SIGNAL(clicked()),
            this, SLOT(slotApply()));
    horizontalLayout->addWidget(radioOgg);
    horizontalLayout->addWidget(radioMp3);

#endif

    //AIFF and WAVE are supported by default
    radioWav = new QRadioButton(ENCODING_WAVE);
    connect(radioWav, SIGNAL(clicked()),
            this, SLOT(slotApply()));
    horizontalLayout->addWidget(radioWav);

    radioAiff = new QRadioButton(ENCODING_AIFF);
    connect(radioAiff, SIGNAL(clicked()),
            this, SLOT(slotApply()));
    horizontalLayout->addWidget(radioAiff);


#ifdef SF_FORMAT_FLAC
    radioFlac = new QRadioButton(ENCODING_FLAC);
    connect(radioFlac,SIGNAL(clicked()),
            this, SLOT(slotApply()));
    horizontalLayout->addWidget(radioFlac);
#endif

    //Read config and check radio button
    QString format = config->getValueString(ConfigKey("[Recording]","Encoding"));
    if(format == ENCODING_WAVE)
        radioWav->setChecked(true);
#ifdef __SHOUTCAST__
    else if(format == ENCODING_OGG)
        radioOgg->setChecked(true);
    else if (format == ENCODING_MP3)
        radioMp3->setChecked(true);
#endif
#ifdef SF_FORMAT_FLAC
    else if (format == ENCODING_AIFF)
        radioAiff->setChecked(true);
#endif
    else //Invalid, so set default and save
    {
        //If no config was available, set to WAVE as default
        radioWav->setChecked(true);
        config->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(ENCODING_WAVE));
    }

    //Connections
    connect(SliderQuality,    SIGNAL(valueChanged(int)), this,  SLOT(slotSliderQuality()));
    connect(SliderQuality,    SIGNAL(sliderMoved(int)), this,   SLOT(slotSliderQuality()));
    connect(SliderQuality,    SIGNAL(sliderReleased()), this,   SLOT(slotSliderQuality()));
    connect(CheckBoxRecordCueFile, SIGNAL(stateChanged(int)), this, SLOT(slotEnableCueFile(int)));
    connect(comboBoxSplitting, SIGNAL(activated(int)),   this,   SLOT(slotChangeSplitSize()));

    slotApply();
    recordControl->slotSet(RECORD_OFF); //make sure a corrupt config file won't cause us to record constantly

    comboBoxSplitting->addItem(SPLIT_650MB);
    comboBoxSplitting->addItem(SPLIT_700MB);
    comboBoxSplitting->addItem(SPLIT_1024MB);
    comboBoxSplitting->addItem(SPLIT_2048MB);
    comboBoxSplitting->addItem(SPLIT_4096MB);

    QString fileSizeStr = config->getValueString(ConfigKey("[Recording]","FileSize"));
    int index = comboBoxSplitting->findText(fileSizeStr);
    if(index > 0){
        //set file split size
        comboBoxSplitting->setCurrentIndex(index);
    }
    //Otherwise 650 MB will be default file split size

    //Read CUEfile info
    CheckBoxRecordCueFile->setChecked((bool) config->getValueString(ConfigKey("[Recording]","CueEnabled")).toInt());

}

void DlgPrefRecord::slotSliderQuality()
{
    updateTextQuality();

    if (radioOgg && radioOgg->isChecked())
    {
        config->set(ConfigKey(RECORDING_PREF_KEY, "OGG_Quality"), ConfigValue(SliderQuality->value()));
    }
    else if (radioMp3 && radioMp3->isChecked())
    {
        config->set(ConfigKey(RECORDING_PREF_KEY, "MP3_Quality"), ConfigValue(SliderQuality->value()));
    }
}

int DlgPrefRecord::getSliderQualityVal()
{

    /* Commented by Tobias Rafreider
     * We always use the bitrate to denote the quality since it is more common to the users
     */
    return Encoder::convertToBitrate(SliderQuality->value());

}

void DlgPrefRecord::updateTextQuality()
{
    int quality = getSliderQualityVal();
    //QString encodingType = comboBoxEncoding->currentText();

    TextQuality->setText(QString( QString::number(quality) + tr("kbps")));


}

void DlgPrefRecord::slotEncoding()
{
    //set defaults
    groupBoxQuality->setEnabled(true);
    //config->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(comboBoxEncoding->currentText()));

    if (radioWav && radioWav->isChecked()) {
        config->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(ENCODING_WAVE));
        groupBoxQuality->setEnabled(false);
    }
    else if(radioFlac && radioFlac->isChecked()){
        config->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(ENCODING_FLAC));
        groupBoxQuality->setEnabled(false);
    }
    else if(radioAiff && radioAiff->isChecked()){
        config->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(ENCODING_AIFF));
        groupBoxQuality->setEnabled(false);
    }
    else if (radioOgg && radioOgg->isChecked())
    {
        int value = config->getValueString(ConfigKey(RECORDING_PREF_KEY, "OGG_Quality")).toInt();
        //if value == 0 then a default value of 128kbps is proposed.
        if(!value)
            value = 6; // 128kbps

        SliderQuality->setValue(value);
        config->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(ENCODING_OGG));
    }
    else if (radioMp3 && radioMp3->isChecked())
    {
        int value = config->getValueString(ConfigKey(RECORDING_PREF_KEY, "MP3_Quality")).toInt();
        //if value == 0 then a default value of 128kbps is proposed.
        if(!value)
            value = 6; // 128kbps

        SliderQuality->setValue(value);
        config->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(ENCODING_MP3));
    }
    else
        qDebug() << "Invalid recording encoding type in" << __FILE__ << "on line:" << __LINE__;
}

void DlgPrefRecord::setMetaData()
{
    config->set(ConfigKey(RECORDING_PREF_KEY, "Title"), ConfigValue(LineEditTitle->text()));
    config->set(ConfigKey(RECORDING_PREF_KEY, "Author"), ConfigValue(LineEditAuthor->text()));
    config->set(ConfigKey(RECORDING_PREF_KEY, "Album"), ConfigValue(LineEditAlbum->text()));
}

void DlgPrefRecord::loadMetaData()
{
    LineEditTitle->setText( config->getValueString(ConfigKey(RECORDING_PREF_KEY, "Title")));
    LineEditAuthor->setText( config->getValueString(ConfigKey(RECORDING_PREF_KEY, "Author")));
    LineEditAlbum->setText( config->getValueString(ConfigKey(RECORDING_PREF_KEY, "Album")));
}

DlgPrefRecord::~DlgPrefRecord()
{
   delete recordControl;
}

void DlgPrefRecord::slotRecordPathChange()
{
    confirmOverwrite = false;
    slotApply();
}

//This function updates/refreshes the contents of this dialog
void DlgPrefRecord::slotUpdate()
{
    // Recordings path
    QString recordingsPath = config->getValueString(ConfigKey("[Recording]","Directory"));
    if (recordingsPath == "") {
        // Initialize recordings path in config to old default path.
        // Do it here so we show current value in UI correctly.
        QDir musicDir(config->getValueString(ConfigKey("[Playlist]","Directory")));
        QDir recordDir(musicDir.absolutePath() + "/Mixxx/Recordings");
        recordingsPath = recordDir.absolutePath();
    }
    LineEditRecordings->setText(recordingsPath);

    if (radioWav && radioWav->isChecked())
    {
        config->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(ENCODING_WAVE));
    }
    else if (radioAiff && radioAiff->isChecked())
    {
        config->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(ENCODING_AIFF));
    }
    else if (radioFlac && radioFlac->isChecked())
    {
        config->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(ENCODING_FLAC));
    }
    else if (radioOgg && radioOgg->isChecked())
    {
        config->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(ENCODING_OGG));
    }
    else if (radioMp3 && radioMp3->isChecked())
    {
       config->set(ConfigKey(RECORDING_PREF_KEY, "Encoding"), ConfigValue(ENCODING_MP3));
    }
    loadMetaData();
}

void DlgPrefRecord::slotBrowseRecordingsDir()
{
    QString fd = QFileDialog::getExistingDirectory(this, tr("Choose recordings direcory"),
                                                   config->getValueString(ConfigKey("[Recording]","Directory")));
    if (fd != "")
    {
        LineEditRecordings->setText(fd);
    }
}

void DlgPrefRecord::slotApply()
{
    setRecordingFolder();

    setMetaData();

    slotEncoding();
}

void DlgPrefRecord::setRecordingFolder() {
    if (LineEditRecordings->text() == "") {
        qDebug() << "Recordings path was empty in dialog";
        return;
    }
    if (LineEditRecordings->text() != config->getValueString(ConfigKey("[Recording]","Directory")))
    {
        qDebug() << "Saved recordings path" << LineEditRecordings->text();
        config->set(ConfigKey("[Recording]","Directory"), LineEditRecordings->text());
    }
}

void DlgPrefRecord::slotEnableCueFile(int enabled)
{
    config->set(ConfigKey(RECORDING_PREF_KEY, "CueEnabled"), ConfigValue(CheckBoxRecordCueFile->isChecked()));

}
void DlgPrefRecord::slotChangeSplitSize()
{
        config->set(ConfigKey(RECORDING_PREF_KEY, "FileSize"), ConfigValue(comboBoxSplitting->currentText()));

}
