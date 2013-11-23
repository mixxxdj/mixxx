/*
 * dlgprefreplaygain.h
 *
 *  Created on: 18/ott/2010
 *      Author: Vittorio Colao
 */

#ifndef DLGPREFREPLAYGAIN_H_
#define DLGPREFREPLAYGAIN_H_
#include "ui_dlgprefreplaygaindlg.h"
#include "configobject.h"
#include "controlobjectthread.h"

class QWidget;

class DlgPrefReplayGain: public QWidget, public Ui::DlgPrefReplayGainDlg  {
    Q_OBJECT
  public:
    DlgPrefReplayGain(QWidget *parent, ConfigObject<ConfigValue> *_config);
    ~DlgPrefReplayGain();
  public slots:
    // Update initial gain increment
    void slotUpdateBoost();
    void slotSetRGEnabled();
    void slotSetRGAnalyserEnabled();

    void slotApply();
    void slotUpdate();
    void setDefaults();
  signals:
    void apply(const QString &);
  private:
    // Determines whether or not to gray out the preferences
    void loadSettings();

    // Pointer to config object
    ConfigObject<ConfigValue> *config;

    ControlObjectThread m_COTInitialBoost;
    ControlObjectThread m_COTEnabled;
};


#endif /* DLGPREFREPLAYGAIN_H_ */
