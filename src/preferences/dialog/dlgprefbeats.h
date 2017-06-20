//  Created on: 28/apr/2011
//      Author: vittorio


#ifndef DLGPREFBEATS_H
#define DLGPREFBEATS_H

#include <QWidget>
#include <QString>
#include <QList>

#include "preferences/usersettings.h"
#include "preferences/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefbeatsdlg.h"

class DlgPrefBeats : public DlgPreferencePage, public Ui::DlgBeatsDlg {
    Q_OBJECT
  public:
    DlgPrefBeats(QWidget *parent, UserSettingsPointer _config);
    virtual ~DlgPrefBeats();

  public slots:
    // Apply changes to widget
    void slotApply();
    void slotUpdate();
    void slotResetToDefaults();

  private slots:
    void pluginSelected(int i);
    void analyzerEnabled(int i);
    void fixedtempoEnabled(int i);
    void offsetEnabled(int i);
    void fastAnalysisEnabled(int i);
    void minBpmRangeChanged(int value);
    void maxBpmRangeChanged(int value);
    void slotReanalyzeChanged(int value);

  private:
    void populate();
    void loadSettings();

    // Pointer to config object
    UserSettingsPointer m_pconfig;
    QList<QString> m_listName;
    QList<QString> m_listLibrary, m_listIdentifier;
    QString m_selectedAnalyzer;
    int m_minBpm;
    int m_maxBpm;
    bool m_banalyzerEnabled, m_bfixedtempoEnabled, m_boffsetEnabled, m_FastAnalysisEnabled, m_bReanalyze;
};

#endif // DLGPREFBEATS_H
