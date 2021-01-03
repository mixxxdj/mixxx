#pragma once

#include <QList>
#include <QMap>
#include <QWidget>

#include "analyzer/plugins/analyzerplugin.h"
#include "control/controlproxy.h"
#include "defs_urls.h"
#include "preferences/dialog/ui_dlgprefkeydlg.h"
#include "preferences/dlgpreferencepage.h"
#include "preferences/keydetectionsettings.h"
#include "preferences/usersettings.h"
#include "track/keyutils.h"

class DlgPrefKey : public DlgPreferencePage, Ui::DlgPrefKeyDlg {
    Q_OBJECT
  public:
    DlgPrefKey(QWidget *parent, UserSettingsPointer _config);
    virtual ~DlgPrefKey();

    QUrl helpUrl() const override;

  public slots:
    // Apply changes to widget
    void slotApply() override;
    void slotUpdate() override;
    void slotResetToDefaults() override;

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
