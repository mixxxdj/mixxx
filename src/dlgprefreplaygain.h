/*
 * dlgprefreplaygain.h
 *
 *  Created on: 18/ott/2010
 *      Author: Vittorio Colao
 */

#ifndef DLGPREFREPLAYGAIN_H_
#define DLGPREFREPLAYGAIN_H_

#include <QWidget>

#include "ui_dlgprefreplaygaindlg.h"
#include "configobject.h"
#include "controlobjectslave.h"
#include "preferences/dlgpreferencepage.h"

class DlgPrefReplayGain: public DlgPreferencePage, public Ui::DlgPrefReplayGainDlg  {
    Q_OBJECT
  public:
    DlgPrefReplayGain(QWidget *parent, ConfigObject<ConfigValue> *_config);
    virtual ~DlgPrefReplayGain();

  public slots:
    // Update initial gain increment
    void slotUpdateBoost();
    void slotSetRGEnabled();
    void slotSetRGAnalyserEnabled();

    void slotApply();
    void slotUpdate();
    void slotResetToDefaults();

  signals:
    void apply(const QString &);

  private:
    // Determines whether or not to gray out the preferences
    void loadSettings();

    // Pointer to config object
    ConfigObject<ConfigValue>* config;

    ControlObjectSlave m_COTReplayGainBoost;
    ControlObjectSlave m_COTEnabled;
};


#endif /* DLGPREFREPLAYGAIN_H_ */
