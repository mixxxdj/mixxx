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

#include "dlgprefrecord.h"
#define MIXXX
#include <qlineedit.h>
#include <qfiledialog.h>
#include <qwidget.h>
#include <qslider.h>
#include <qlabel.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qcheckbox.h>

DlgPrefRecord::DlgPrefRecord(QWidget *parent, ConfigObject<ConfigValue> *_config) : DlgPrefRecordDlg(parent,"")
{
    config = _config;
    confirmOverwrite = false;

    //Fill up encoding list
    comboBoxEncoding->insertItem("WAVE", IDEX_WAVE);
    comboBoxEncoding->insertItem("FLAC", IDEX_FLAC);
    comboBoxEncoding->insertItem("AIFF", IDEX_AIFF);
    //comboBoxEncoding->insertItem("OGG",  IDEX_OGG);
    //comboBoxEncoding->insertItem("MP3",  IDEX_MP3);


    //Connections
    connect(PushButtonBrowse, SIGNAL(clicked()),	this,	SLOT(slotBrowseSave()));
    connect(LineEditRecPath,  SIGNAL(returnPressed()),  this,	SLOT(slotApply()));
    connect(comboBoxEncoding, SIGNAL(activated(int)),	this,	SLOT(slotRecordPathChange()));
    connect(SliderQuality,    SIGNAL(valueChanged(int)), this,	SLOT(slotSliderQuality()));
    connect(SliderQuality,    SIGNAL(sliderMoved(int)),	this,	SLOT(slotSliderQuality()));
    connect(SliderQuality,    SIGNAL(sliderReleased()), this,	SLOT(slotSliderQuality()));
    //connect(CheckBoxRecord,   SIGNAL(stateChanged(int)),this,	SLOT(slotApply()));
    connect(CheckBoxRecord,            SIGNAL(stateChanged(int)), this, SLOT(slotApply()));


    loadMetaData();
    slotApply();
    config->set(ConfigKey(PREF_KEY, "Record"), QString("FALSE"));//make sure a corrupt config file won't cause us to record constantly
}

void DlgPrefRecord::slotBrowseSave()
{
    QFileDialog* fd = new QFileDialog(config->getValueString(ConfigKey(PREF_KEY,"Path")),"", this, "", TRUE );
    fd->setMode( QFileDialog::AnyFile );
    fd->setCaption("Save As");
    if ( fd->exec() == QDialog::Accepted )
    {
	QString selectedFile = fd->selectedFile();
	if(!selectedFile.endsWith("." + fileTypeExtension))
	{
	    selectedFile.append("." + fileTypeExtension);
	}
	LineEditRecPath->setText( selectedFile );
    }
}

void DlgPrefRecord::slotSliderQuality()
{
    updateTextQuality();
    switch(comboBoxEncoding->currentItem())
    {
	case IDEX_OGG:
	    config->set(ConfigKey(PREF_KEY, "OGG_Quality"), ConfigValue(SliderQuality->value()));
	    break;
	case IDEX_MP3:
	    config->set(ConfigKey(PREF_KEY, "MP3_Quality"), ConfigValue(SliderQuality->value()));
	    break;
    }
}

int DlgPrefRecord::getSliderQualityVal()
{
    switch(comboBoxEncoding->currentItem())
    {
	case IDEX_OGG:
	    return SliderQuality->value();
	case IDEX_MP3:
	    switch(SliderQuality->value())
	    {
		case 1:	return 16;
		case 2: return 24;
		case 3: return 32;
		case 4: return 64;
		case 5: return 128;
		case 6: return 160;
		case 7: return 192;
		case 8: return 224;
		case 9: return 256;
		case 10: return 320;
	    }
    }
    return 0;
}		    

void DlgPrefRecord::updateTextQuality()
{
    int quality = getSliderQualityVal();
    switch(comboBoxEncoding->currentItem())
    {
	case IDEX_MP3:
	    TextQuality->setText(QString( QString::number(quality) + "kbps"));
	    break;
	case IDEX_OGG:
	    TextQuality->setText(QString( "Quality: " + QString::number(quality)));
	    break;
    }
}

void DlgPrefRecord::slotEncoding()
{
    //set defaults
    groupBoxQuality->setEnabled(true);
    config->set(ConfigKey(PREF_KEY, "Encoding"), ConfigValue(comboBoxEncoding->currentItem()));
    switch(comboBoxEncoding->currentItem())
    {
	case IDEX_WAVE:
	    groupBoxQuality->setEnabled(false);
	    fileTypeExtension = QString("wav");
	    break;
	
	case IDEX_FLAC:
	    groupBoxQuality->setEnabled(false);
	    fileTypeExtension = QString("flac");
	    break;

	case IDEX_OGG:
	    SliderQuality->setValue( config->getValueString(ConfigKey(PREF_KEY, "OGG_Quality")).toInt());
	    fileTypeExtension = QString("ogg");
	    break;

	case IDEX_MP3:
	    SliderQuality->setValue( config->getValueString(ConfigKey(PREF_KEY, "MP3_Quality")).toInt());
	    fileTypeExtension = QString("mp3");
	    break;

	case IDEX_AIFF:
	    groupBoxQuality->setEnabled(false);
	    fileTypeExtension = QString("aiff");
	    break;
    }
}
   
void DlgPrefRecord::setMetaData()
{
    config->set(ConfigKey(PREF_KEY, "Title"), ConfigValue(LineEditTitle->text()));
    config->set(ConfigKey(PREF_KEY, "Author"), ConfigValue(LineEditTitle->text()));
    config->set(ConfigKey(PREF_KEY, "Album"), ConfigValue(LineEditAlbum->text()));
}

void DlgPrefRecord::loadMetaData()
{
    LineEditTitle->setText( config->getValueString(ConfigKey(PREF_KEY, "Title")));
    LineEditAuthor->setText( config->getValueString(ConfigKey(PREF_KEY, "Author")));
    LineEditAlbum->setText( config->getValueString(ConfigKey(PREF_KEY, "Album")));
}

DlgPrefRecord::~DlgPrefRecord()
{

}
void DlgPrefRecord::slotRecordPathChange()
{
    confirmOverwrite = false;
    CheckBoxRecord->setChecked(false);
    slotApply();
}

void DlgPrefRecord::slotApply()
{

    config->set(ConfigKey(PREF_KEY, "Path"), LineEditRecPath->text());
    setMetaData();
    

    slotEncoding();

    if(CheckBoxRecord->isOn())
    {
	int result = 0;
	if(QFile::exists(LineEditRecPath->text()) && !confirmOverwrite){
	    result = QMessageBox::warning( this, "Mixxx Recording", "The selected file already exists, would you like to overwrite it?\n\n\tNote: Selecting No will abort the recording", "Yes", "No", 0, 0, 1);
	}
	if(result == 0)
	{
	    qDebug("Setting record status: READY");
	    confirmOverwrite = true;
	    config->set(ConfigKey(PREF_KEY, "Record"), QString("READY"));
	    /*
	     * Note: setting the Record value to READY does not start the
	     * recording.  The READY flag signals code elsewhere that when
	     * its condition is met (first track is played), the flag can
	     * be set to TRUE
	     *
	     */
	}
	else
	{
	    CheckBoxRecord->setChecked(false);
	}
    }
    else
    {
	qDebug("Setting record status: FALSE");
	config->set(ConfigKey(PREF_KEY, "Record"), QString("FALSE"));
    }
}
