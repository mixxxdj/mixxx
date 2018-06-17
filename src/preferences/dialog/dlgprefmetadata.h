#pragma once

#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefmetadatadlg.h"

class DlgPrefMetadata : public DlgPreferencePage, public Ui::DlgPrefMetadataDlg {
  public:
    DlgPrefMetadata(QWidget *pParent);

  public slots:
    void slotApply() override;
    void slotCancel() override;
    void slotResetToDefaults() override;
    void slotUpdate() override;
};
















