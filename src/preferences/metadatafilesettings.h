#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QObject>
#include <QPushButton>
#include <QStyledItemDelegate>

#include "control/controlproxy.h"
#include "preferences/usersettings.h"

namespace {
const ConfigKey kMetadataFileEnabled =
        ConfigKey("[Livemetadata]", "MetadataFileEnabled");

const ConfigKey kFileEncoding =
        ConfigKey("[Livemetadata]", "FileEncoding");

const ConfigKey kFileFormatString =
        ConfigKey("[Livemetadata]", "FileFormatString");

const ConfigKey kFilePath =
        ConfigKey("[Livemetadata]", "CustomFormatString");
const ConfigKey kFileSettingsChanged =
        ConfigKey("[Livemetadata]", "FileSettingsChanged");

const bool defaultFileMetadataEnabled = false;
const QByteArray defaultEncoding = "UTF-8";
const QString defaultFilePath = QDir::currentPath() + "/NowPlaying.txt";
const QString defaultFileFormatString = "{{ track.artist }} - {{ track.title }}";
} // namespace

struct FileSettings {
    bool enabled;
    QByteArray fileEncoding;
    QString fileFormatString, filePath;
};

struct FileWidgets {
    QCheckBox* enableCheckbox;
    QComboBox* encodingBox;
    QLineEdit *formatLineEdit,
            *filePathLineEdit;
    QPushButton* changeFilePathButton;
};

class MetadataFileSettings : public QObject {
    Q_OBJECT
  public:
    MetadataFileSettings(UserSettingsPointer pSettings,
            const FileWidgets& widgets,
            QWidget* dialogWidget);
    ~MetadataFileSettings() = default;
    static FileSettings getLatestSettings();
    static FileSettings getPersistedSettings(const UserSettingsPointer& pSettings);
    void applySettings();
    void cancelSettings();
    void setSettingsToDefault();

  private:
    void setupWidgets();
    void setupEncodingComboBox();
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

    const QSet<QByteArray> m_fileEncodings;

  private slots:
    void slotFilepathButtonClicked();
};
