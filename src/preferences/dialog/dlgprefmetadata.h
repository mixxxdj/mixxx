#pragma once

#include "broadcast/scrobblingservice.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefmetadatadlg.h"
#include "preferences/usersettings.h"

namespace Ui {
class fileListenerBox;
}

class DlgPrefMetadata : public DlgPreferencePage, public Ui::DlgPrefMetadataDlg {
    Q_OBJECT
  public:
    DlgPrefMetadata(QWidget* pParent, UserSettingsPointer pSettings);
    ~DlgPrefMetadata() override;

  public slots:
    void slotApply() override;
    void slotCancel() override;
    void slotResetToDefaults() override;
    void slotUpdate() override;

  private:
    void setupTableWidget(UserSettingsPointer pSettings);
    void fillTableWithServices(const QLinkedList<ScrobblingServicePtr>& listeners);
    Ui::fileListenerBox* fileListenerBox;
  private slots:
    void slotCurrentListenerChanged(const QModelIndex& previous, const QModelIndex& current);
};
