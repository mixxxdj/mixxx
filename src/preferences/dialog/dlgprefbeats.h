#pragma once

#include <QWidget>
#include <QString>
#include <QList>

#include "analyzer/plugins/analyzerplugin.h"
#include "preferences/beatdetectionsettings.h"
#include "preferences/usersettings.h"
#include "preferences/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefbeatsdlg.h"

class DlgPrefBeats : public DlgPreferencePage, public Ui::DlgBeatsDlg {
    Q_OBJECT
  public:
    DlgPrefBeats(QWidget *parent, UserSettingsPointer _config);
    virtual ~DlgPrefBeats();

    QUrl helpUrl() const override;

  public slots:
    // Apply changes to widget
    void slotApply() override;
    void slotUpdate() override;
    void slotResetToDefaults() override;

  private slots:
    void pluginSelected(int i);
    void analyzerEnabled(int i);
    void fixedtempoEnabled(int i);
    void offsetEnabled(int i);
    void fastAnalysisEnabled(int i);
    void minBpmRangeChanged(int value);
    void maxBpmRangeChanged(int value);
    void slotReanalyzeChanged(int value);
    void slotReanalyzeImportedChanged(int value);

  private:
    void loadSettings();

    BeatDetectionSettings m_bpmSettings;
    QList<mixxx::AnalyzerPluginInfo> m_availablePlugins;
    QString m_selectedAnalyzerId;
    int m_minBpm;
    int m_maxBpm;
    bool m_bAnalyzerEnabled;
    bool m_bFixedTempoEnabled;
    bool m_bOffsetEnabled;
    bool m_bFastAnalysisEnabled;
    bool m_bReanalyze;
    bool m_bReanalyzeImported;
};
