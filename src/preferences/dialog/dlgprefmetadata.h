#pragma once

#include "broadcast/scrobblingservice.h"
#include "preferences/usersettings.h"
#include "preferences/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefmetadatadlg.h"

namespace Ui {
    class fileListenerBox;
}

class DlgPrefMetadata : public DlgPreferencePage, public Ui::DlgPrefMetadataDlg {
    Q_OBJECT
  public:
    DlgPrefMetadata(QWidget *pParent, UserSettingsPointer pSettings);
    ~DlgPrefMetadata() override;
  private:
    void setupTableWidget(UserSettingsPointer pSettings);
    void setTableParameters(int numberOfListeners);
    void fillTableWithServices(const QLinkedList<ScrobblingServicePtr> &listeners);
    Ui::fileListenerBox *fileListenerBox;
  private slots:
    void slotCurrentListenerChanged(const QModelIndex &previous, const QModelIndex &current);
};



