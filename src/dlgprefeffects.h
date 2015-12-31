#ifndef DLGPREFEFFECTS_H
#define DLGPREFEFFECTS_H

#include "configobject.h"
#include "ui_dlgprefeffectsdlg.h"
#include "preferences/dlgpreferencepage.h"

class EffectsManager;

class DlgPrefEffects : public DlgPreferencePage, public Ui::DlgPrefEffectsDlg {
    Q_OBJECT
  public:
    DlgPrefEffects(QWidget* pParent,
                   ConfigObject<ConfigValue>* pConfig,
                   EffectsManager* pEffectsManager);
    virtual ~DlgPrefEffects() {}

    void slotUpdate();
    void slotApply();
    void slotResetToDefaults();

  private slots:
    void slotEffectSelected(QListWidgetItem* pCurrent,
                            QListWidgetItem* pPrevious);

  private:
    void addEffectToList(const QString& id);
    void clear();

    ConfigObject<ConfigValue>* m_pConfig;
    EffectsManager* m_pEffectsManager;
};

#endif /* DLGPREFEFFECTS_H */
