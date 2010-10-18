/*
 * dlgprefreplaygain.h
 *
 *  Created on: 18/ott/2010
 *      Author: vittorio
 */

#ifndef DLGPREFREPLAYGAIN_H_
#define DLGPREFREPLAYGAIN_H_
#include "ui_dlgprefreplaygaindlg.h"
#include "configobject.h"


class QWidget;

class DlgPrefReplayGain: public QWidget, public Ui::DlgPrefReplayGainDlg  {
    Q_OBJECT
public:
    DlgPrefReplayGain(QWidget *parent, ConfigObject<ConfigValue> *_config);
    ~DlgPrefReplayGain();
public slots:
	/** Update initial gain increment */
	void slotUpdateBoost(int);
    /** Apply changes to widget */
	void slotSetRGEnabled(int);
	void slotSetRGAnalyserEnabled(int);
	//void slotApply();
	//void slotUpdate();
	void setDefaults();
signals:
    void apply(const QString &);
private:

    // Determines whether or not to gray out the preferences
    void loadSettings();
    void updateRGEnabled();

    /** Pointer to config object */
    ConfigObject<ConfigValue> *config;

};


#endif /* DLGPREFREPLAYGAIN_H_ */
