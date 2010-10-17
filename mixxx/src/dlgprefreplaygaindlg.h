
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGPREFBPM_H
#define DLGPREFBPM_H

#include "ui_dlgreplaygaindlg.h"
#include "configobject.h"

#include <qlist.h>











/*#define MIXXX_XFADER_SLOWFADE   "SlowFade"
#define MIXXX_XFADER_FASTCUT    "FastCut"
#define MIXXX_XFADER_CONSTPWR   "ConstantPower"

#define MIXXX_XFADER_STEEPNESS_COEFF 8.0f**/



class QWidget;

class DlgPrefRG : public QWidget, public Ui::Ui_DlgPrefReplayGainDlg  {
    Q_OBJECT
public: 
    DlgPrefRG(QWidget *parent, ConfigObject<ConfigValue> *_config);
    ~DlgPrefRGr();
public slots:
	/** Update Boost */
	void slotUpdateBoost();
	/** Enable ReplayGain */ 
	void slotSetRGEnabled(int);
	/** Enable analyserg */
	void slotSetRGanalyser(int)
    /** Apply changes to widget */
    void slotApply();
	void slotUpdate();
	void setDefaults();
signals:
    void apply(const QString &);
private:
	void loadSettings();
	
    /** Pointer to config object */
    ConfigObject<ConfigValue> *config;

	/** X-fader values */
	double m_transform, m_cal;
	
	/** X-fader mode*/
	QString m_GainMode;
};

#endif

