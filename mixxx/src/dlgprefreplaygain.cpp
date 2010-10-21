/*
 * dlgprefreplaygaindlg.cpp
 *
 *  Created on: 18/ott/2010
 *      Author: vittorio
 */

#include <qlineedit.h>
#include <qwidget.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <QtCore>
#include <QMessageBox>
#include "controlobject.h"

#include "dlgprefreplaygain.h"

#define CONFIG_KEY "[ReplayGain]"


DlgPrefReplayGain::DlgPrefReplayGain(QWidget * parent, ConfigObject<ConfigValue> * _config) :  QWidget(parent), Ui::DlgPrefReplayGainDlg()
{

	config = _config;


	setupUi(this);

	//Connections
	    connect(EnableGain,      SIGNAL(stateChanged(int)), this, SLOT(slotSetRGEnabled()));
	    connect(EnableAnalyser,  SIGNAL(stateChanged(int)), this, SLOT(slotSetRGAnalyserEnabled()));
	    connect(SliderBoost,     SIGNAL(valueChanged(int)), this, SLOT(slotUpdateBoost()));
	    connect(SliderBoost,     SIGNAL(sliderReleased()),  this, SLOT(slotApply()));
	    connect(PushButtonReset, SIGNAL(clicked(bool)),     this, SLOT(setDefaults()));

	loadSettings();
}

DlgPrefReplayGain::~DlgPrefReplayGain()
{
}

void DlgPrefReplayGain::loadSettings()
{
	if(config->getValueString(ConfigKey(CONFIG_KEY,"ReplayGainEnabled"))==QString(""))
			setDefaults();
	else
	{
		SliderBoost->setValue(config->getValueString(ConfigKey(CONFIG_KEY, "InitialReplayGainBoost")).toInt());
		EnableGain->setChecked(false);
		if(config->getValueString(ConfigKey(CONFIG_KEY, "ReplayGainEnabled")).toInt()==1)EnableGain->setChecked(true);
		EnableAnalyser->setChecked(false);
		if(config->getValueString(ConfigKey(CONFIG_KEY, "ReplayGainAnalyserEnabled")).toInt())EnableAnalyser->setChecked(true);
		slotSetRGAnalyserEnabled();
		slotSetRGEnabled();
		slotUpdateBoost();
	}
	slotUpdate();
	slotApply();
}

void DlgPrefReplayGain::setDefaults()
{
	EnableGain->setChecked(true);
	EnableAnalyser->setChecked(false);
	SliderBoost->setValue(6);

	slotUpdate();
	slotApply();
}


void DlgPrefReplayGain::slotSetRGEnabled()
{
	if (EnableGain->isChecked())
		config->set(ConfigKey(CONFIG_KEY,"ReplayGainEnabled"), ConfigValue(1));

	else
	{
		config->set(ConfigKey(CONFIG_KEY,"ReplayGainEnabled"), ConfigValue(0));
		config->set(ConfigKey(CONFIG_KEY,"ReplayGainAnalyserEnabled"), ConfigValue(0));
	}


	slotUpdate();
	slotApply();
}

void DlgPrefReplayGain::slotSetRGAnalyserEnabled()
{
	if (EnableAnalyser->isChecked())
		config->set(ConfigKey(CONFIG_KEY,"ReplayGainAnalyserEnabled"), ConfigValue(1));
		else
		config->set(ConfigKey(CONFIG_KEY,"ReplayGainAnalyserEnabled"), ConfigValue(0));
	slotApply();
}


void DlgPrefReplayGain::slotUpdateBoost()
{
	config->set(ConfigKey(CONFIG_KEY, "InitialReplayGainBoost"), ConfigValue(SliderBoost->value()));
	slotApply();
}


 void DlgPrefReplayGain::slotUpdate()
 {
	 if (config->getValueString(ConfigKey(CONFIG_KEY,"ReplayGainEnabled")).toInt()==1)
	     {
	     	EnableAnalyser->setEnabled(true);
	     	SliderBoost->setEnabled(true);
	     }
	     else
	     {
	     	EnableAnalyser->setChecked(false);
	     	EnableAnalyser->setEnabled(false);
	     	SliderBoost->setValue(0);
	     	SliderBoost->setEnabled(false);
	     }
 }

 void DlgPrefReplayGain::slotApply()
 {
 	ControlObject::getControl(ConfigKey(CONFIG_KEY, "InitialReplayGainBoost"))->set(SliderBoost->value());
 	int iRGenabled = 0;
 	int iRGAnalyserEnabled = 0;
 	if (EnableGain->isChecked()) iRGenabled = 1;
 	if (EnableAnalyser->isChecked()) iRGAnalyserEnabled = 1;
 	ControlObject::getControl(ConfigKey(CONFIG_KEY, "ReplayGainEnabled"))->set(iRGenabled);
 }

