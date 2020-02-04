#ifndef DLGPREFKEY_H
#define DLGPREFKEY_H

#include <QList>
#include <QWidget>
#include <QMap>

#include "analyzer/plugins/analyzerplugin.h"
#include "control/controlproxy.h"
#include "preferences/dialog/ui_dlgprefkeydlg.h"
#include "preferences/keydetectionsettings.h"
#include "preferences/usersettings.h"
#include "track/keyutils.h"
#include "preferences/dlgpreferencepage.h"

class DlgPrefKey : public DlgPreferencePage, Ui::DlgPrefKeyDlg {
    Q_OBJECT
  public:
    DlgPrefKey(QWidget *parent, UserSettingsPointer _config);
    virtual ~DlgPrefKey();

  public slots:
    // Apply changes to widget
    void slotApply();
    void slotUpdate();
    void slotResetToDefaults();

  private slots:
    void pluginSelected(int i);
    void analyzerEnabled(int i);
    void fastAnalysisEnabled(int i);
    void reanalyzeEnabled(int i);

    void setNotation(KeyUtils::KeyNotation notation);
    void setNotationOpenKey(bool);
    void setNotationOpenKeyAndTraditional(bool);
    void setNotationLancelot(bool);
    void setNotationLancelotAndTraditional(bool);
    void setNotationTraditional(bool);
    void setNotationCustom(bool);

  private:
    void loadSettings();

    KeyDetectionSettings m_keySettings;
    QMap<mixxx::track::io::key::ChromaticKey, QLineEdit*> m_keyLineEdits;
    QList<mixxx::AnalyzerPluginInfo> m_availablePlugins;
    QString m_selectedAnalyzerId;
    ControlProxy* m_pKeyNotation;
    bool m_bAnalyzerEnabled;
    bool m_bFastAnalysisEnabled;
    bool m_bReanalyzeEnabled;
};

#endif
