#ifndef DLGPREFLV2_H
#define DLGPREFLV2_H

#include <QWidget>
#include <QCheckBox>

#include "effects/effectsmanager.h"
#include "effects/lv2/lv2backend.h"
#include "preferences/dialog/ui_dlgpreflv2dlg.h"
#include "preferences/usersettings.h"
#include "preferences/dlgpreferencepage.h"

class DlgPrefLV2 : public DlgPreferencePage, public Ui::DlgPrefLV2Dlg  {
    Q_OBJECT
  public:
    DlgPrefLV2(QWidget *parent, UserSettingsPointer pConfig,
               std::shared_ptr<EffectsManager> pEffectsManager);
    virtual ~DlgPrefLV2();

  public slots:
    void slotApply();

  private slots:
    void slotDisplayParameters();
    void slotUpdateOnParameterCheck(int state);

  private:
    LV2Backend* m_pLV2Backend;
    QString m_currentEffectId;
    QList<QCheckBox*> m_pluginParameters;
    int m_iCheckedParameters;
    std::shared_ptr<EffectsManager> m_pEffectsManager;
};

#endif
