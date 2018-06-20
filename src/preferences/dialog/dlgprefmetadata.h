#pragma once

#include "broadcast/scrobblingservice.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefmetadatadlg.h"
#include "preferences/usersettings.h"

namespace Ui {
class fileListenerBox;
}

namespace {
const ConfigKey kMetadataFileEnabled =
        ConfigKey("[Livemetadata]", "MetadataFileEnabled");

const ConfigKey kFileEncoding =
        ConfigKey("[Livemetadata]", "FileEncoding");

const ConfigKey kFileFormat =
        ConfigKey("[Livemetadata]", "FileFormat");

const ConfigKey kFileFormatString =
        ConfigKey("[Livemetadata]", "FileFormatString");

const ConfigKey kFilePath =
        ConfigKey("[Livemetadata]", "CustomFormatString");
const ConfigKey kSettingsChanged =
        ConfigKey("[Livemetadata]", "SettingsChanged");

    const bool defaultFileMetadataEnabled = false;
    const QByteArray defaultEncoding = "UTF-8";
    const QString defaultFileFormat = "SAMBroadcaster";
    const QString defaultFilePath = QDir::currentPath();
    const QString defaultFileFormatString = "author - title";
};

struct FileSettings {
    bool enabled;
    QByteArray fileEncoding;
    QString fileFormat, fileFormatString, filePath;
};

class DlgPrefMetadata : public DlgPreferencePage, public Ui::DlgPrefMetadataDlg {
    Q_OBJECT
  public:
    DlgPrefMetadata(QWidget* pParent, UserSettingsPointer pSettings);
    ~DlgPrefMetadata() override = default;
    static FileSettings getLatestSettings();
    static FileSettings getPersistedSettings(const UserSettingsPointer &pSettings);
  public slots:
    void slotApply() override;
    void slotCancel() override;
    void slotResetToDefaults() override;
    void slotUpdate() override;

  private:
    void setupWidgets();
    bool fileSettingsDifferent();
    bool checkIfSettingsCorrect();
    void saveLatestSettingsAndNotify();
    void persistSettings();
    void resetSettingsToDefault();

    UserSettingsPointer m_pSettings;
    ControlProxy m_CPSettingsChanged;
    static FileSettings s_latestSettings;
  private slots:
    void slotFormatChanged(int newIndex);
    void slotFilepathButtonClicked();
};
