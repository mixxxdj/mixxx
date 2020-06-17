#pragma once

#include <QList>
#include <QString>
#include <QWidget>

#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefrhythmdlg.h"
#include "preferences/usersettings.h"

class DlgPrefRhythm : public DlgPreferencePage, public Ui::DlgPrefRhythm {
    Q_OBJECT
  public:
    DlgPrefRhythm(QWidget* parent, UserSettingsPointer _config);
    virtual ~DlgPrefRhythm();

    QUrl helpUrl() const override;

  public slots:
    // Apply changes to widget
    void slotApply() override;
    void slotUpdate() override;
    void slotResetToDefaults() override;

  private slots:
    void analyzerEnabled(int i);
    void minBpmRangeChanged(int value);
    void maxBpmRangeChanged(int value);

  private:
    void loadSettings();

    bool m_banalyzerEnabled;
    int m_minBpm;
    int m_maxBpm;
};
