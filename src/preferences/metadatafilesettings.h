#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QObject>
#include <QPushButton>
#include <QStyledItemDelegate>

#include "control/controlproxy.h"
#include "preferences/usersettings.h"

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

    static const ConfigKey kFileSettingsChanged;

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
