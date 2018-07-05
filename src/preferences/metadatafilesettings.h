#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QObject>
#include <QPushButton>

#include "control/controlproxy.h"
#include "preferences/usersettings.h"

namespace {
const ConfigKey kMetadataFileEnabled =
        ConfigKey("[Livemetadata]", "MetadataFileEnabled");

const ConfigKey kFileEncoding =
        ConfigKey("[Livemetadata]", "FileEncoding");

const ConfigKey kFileFormat =
        ConfigKey("[Livemetadata]", "FileFormat");

const ConfigKey kFileFormatString =
        ConfigKey("[Livemetadata]", "FileFormatString");

    const ConfigKey kFilePath  =
            ConfigKey("[Livemetadata]","CustomFormatString");
    const ConfigKey kFileSettingsChanged =
            ConfigKey("[Livemetadata]","FileSettingsChanged");

const bool defaultFileMetadataEnabled = false;
const QByteArray defaultEncoding = "UTF-8";
const QString defaultFileFormat = "SAMBroadcaster";
const QString defaultFilePath = QDir::currentPath() + "/NowPlaying.txt";
const QString defaultFileFormatString = "$author - $title";
} // namespace

struct FileSettings {
    bool enabled;
    QByteArray fileEncoding;
    QString fileFormat, fileFormatString, filePath;
};

struct FileWidgets {
    QCheckBox *enableCheckbox, *enableCustomFormatBox;
    QComboBox *encodingBox, *formatBox;
    QLineEdit *formatLineEdit,
            *customFormatLineEdit,
            *filePathLineEdit;
    QPushButton* changeFilePathButton;
};

class MetadataFileSettings : public QObject {
    Q_OBJECT
  public:
    MetadataFileSettings(UserSettingsPointer pSettings,
            const FileWidgets& widgets,
            QWidget* dialogWidget);
    static FileSettings getLatestSettings();
    static FileSettings getPersistedSettings(const UserSettingsPointer& pSettings);
    void applySettings();
    void cancelSettings();
    void setSettingsToDefault();

  private:
    void setupWidgets();
    void updateLatestSettingsAndNotify();
    void persistSettings();
    void resetSettingsToDefault();
    bool fileSettingsDifferent();
    bool checkIfSettingsCorrect();

    UserSettingsPointer m_pSettings;
    ControlProxy m_CPSettingsChanged;
    static FileSettings s_latestSettings;
    FileWidgets m_widgets;
    QWidget* m_pDialogWidget;
  private slots:
    void slotFormatChanged(int newIndex);
    void slotFilepathButtonClicked();
};
