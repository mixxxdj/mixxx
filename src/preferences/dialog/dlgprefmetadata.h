#pragma once

#include "preferences/metadatafilesettings.h"
#include "broadcast/scrobblingservice.h"
#include "preferences/usersettings.h"
#include "preferences/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefmetadatadlg.h"

namespace Ui {
    class fileListenerBox;
}

namespace {


    const ConfigKey kListenbrainzEnabled =
            ConfigKey("[Livemetadata]","ListenbrainzEnabled");


};



class DlgPrefMetadata : public DlgPreferencePage, public Ui::DlgPrefMetadataDlg {
  Q_OBJECT
  public:
    DlgPrefMetadata(QWidget *pParent, UserSettingsPointer pSettings);
    ~DlgPrefMetadata();
  public slots:
    void slotApply() override;
    void slotCancel() override;
    void slotResetToDefaults() override;
  private:
    UserSettingsPointer m_pSettings;
    MetadataFileSettings *m_pFileSettings;
    void setFileSettings();
};



