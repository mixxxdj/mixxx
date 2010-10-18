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

#include "dlgprefreplaygain.h"

#define CONFIG_KEY "[ReplayGain]"


DlgPrefReplayGain::DlgPrefReplayGain(QWidget * parent, ConfigObject<ConfigValue> * _config) :  QWidget(parent), Ui::DlgPrefReplayGainDlg()
{

	config = _config;


	setupUi(this);

	//Connections
	    connect(EnableGain,      SIGNAL(stateChanged(int)), this, SLOT(slotSetRGEnabled(int)));
	    connect(EnableAnalyser,  SIGNAL(stateChanged(int)), this, SLOT(slotSetRGAnalyserEnabled(int)));
	    connect(SliderBoost,     SIGNAL(valueChanged(int)), this, SLOT(slotUpdateBoost(int)));
	    //connect(SliderBoost,     SIGNAL(sliderMoved(int)),  this, SLOT(slotUpdateBoost(int)));
	    //connect(SliderBoost,     SIGNAL(sliderReleased()),  this, SLOT(slotUpdateBoost(int)));
	    connect(PushButtonReset, SIGNAL(clicked(bool)),     this, SLOT(setDefaults()));

	loadSettings();
	}

DlgPrefReplayGain::~DlgPrefReplayGain()
	{
	}

void DlgPrefReplayGain::loadSettings()
	{	// Determine if the config value has already been set. If not, set it to true;
    QString sRGEnabled = config->getValueString(ConfigKey(CONFIG_KEY,"ReplayGainEnabled"));
    if(sRGEnabled.isNull() || sRGEnabled.isEmpty())
    {
    	config->set(ConfigKey(CONFIG_KEY,"ReplayGainEnabled"), ConfigValue(1));
    }
	    // Set default value for analyze mode check box
	    int iRGEnabled = config->getValueString(ConfigKey(CONFIG_KEY,"ReplayGainEnabled")).toInt();
	    if (iRGEnabled)
	    	EnableGain->setChecked(true);
	    else
	    	{
	    	EnableGain->setChecked(false);
	    	EnableAnalyser->setChecked(false);
	    	}

	    int iEnableAnalyser = config->getValueString(ConfigKey(CONFIG_KEY,"ReplayGainAnalyserEnabled")).toInt();
	    if (iEnableAnalyser)
	    	EnableAnalyser->setChecked(true);
	    else
	    	EnableAnalyser->setChecked(false);

	    int iGainBoost = config ->getValueString(ConfigKey(CONFIG_KEY,"InitialReplayGainBoost")).toInt();
	    SliderBoost->setValue(iGainBoost);

	    updateRGEnabled();
	}

void DlgPrefReplayGain::updateRGEnabled()
	{
    	int iRGEnabled = config->getValueString(ConfigKey(CONFIG_KEY,"ReplayGainEnabled")).toInt();
    	if (iRGEnabled)
    	{
    		EnableAnalyser->setEnabled(true);
    		SliderBoost->setEnabled(true);
    	}
    	else
    	{
    		EnableAnalyser->setChecked(false);
    		EnableAnalyser->setEnabled(false);
    		SliderBoost->setEnabled(false);
    	}

	}

void DlgPrefReplayGain::slotSetRGEnabled(int)
	{
		if (EnableGain->isChecked())
			config->set(ConfigKey(CONFIG_KEY,"ReplayGainEnabled"), ConfigValue(1));
		else
		{
			config->set(ConfigKey(CONFIG_KEY,"ReplayGainEnabled"), ConfigValue(0));
			config->set(ConfigKey(CONFIG_KEY,"ReplayGainAnalyserEnabled"), ConfigValue(0));
		}
		updateRGEnabled();

}

void DlgPrefReplayGain::slotSetRGAnalyserEnabled(int)
	{
	if (EnableGain->isChecked())
			config->set(ConfigKey(CONFIG_KEY,"ReplayGainAnalyserEnabled"), ConfigValue(1));
		else
			config->set(ConfigKey(CONFIG_KEY,"ReplayGainAnalyserEnabled"), ConfigValue(0));
	}

void DlgPrefReplayGain::slotUpdateBoost(int BoostValue)
	{
		config->set(ConfigKey(CONFIG_KEY,"InitialReplayGainBoost"),ConfigValue(BoostValue));
	}

void DlgPrefReplayGain::setDefaults()
	{
		EnableGain->setChecked(true);
		EnableAnalyser->setChecked(false);
		SliderBoost->setValue(6);
	}

