#pragma once

#include <QWidget>
#include <QString>
#include <QList>

#include "preferences/usersettings.h"
#include "preferences/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefrhythm.h"

class DlgPrefRhythm : public DlgPreferencePage, public Ui::DlgPrefRhythm {
    Q_OBJECT
  public:
    DlgPrefRhythm(QWidget *parent, UserSettingsPointer _config);
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