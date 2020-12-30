#pragma once

#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefmetadatadlg.h"
#include "preferences/usersettings.h"

class MetadataFileSettings;
class ListenBrainzSettingsManager;

/// Dialog class for the metadata broadcasting preferences
class DlgPrefMetadata : public DlgPreferencePage, public Ui::DlgPrefMetadataDlg {
    Q_OBJECT
  public:
    DlgPrefMetadata(QWidget* pParent, const UserSettingsPointer& pSettings);
    ~DlgPrefMetadata() override;
  public slots:
    void slotApply() override;
    void slotCancel() override;
    void slotResetToDefaults() override;
    void slotUpdate() override;

  private:
    UserSettingsPointer m_pSettings;
    MetadataFileSettings* m_pFileSettings;
    ListenBrainzSettingsManager* m_pListenBrainzSettings;
    void setFileSettings();

    void setListenBrainzSettings();
};
