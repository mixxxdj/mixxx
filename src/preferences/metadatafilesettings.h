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
const QString defaultFileFormatString = "$author - $title";
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

class ComboboxDelegate : public QStyledItemDelegate {
    Q_OBJECT
  public:
    void paint(QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

  protected:
    bool editorEvent(QEvent* event,
            QAbstractItemModel* model,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) override;
  signals:
    void moreButtonPressed();
};

class MetadataFileSettings : public QObject {
    Q_OBJECT
  public:
    MetadataFileSettings(UserSettingsPointer pSettings,
            const FileWidgets& widgets,
            QWidget* dialogWidget);
    ~MetadataFileSettings();
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
    ComboboxDelegate* m_pDelegate;
    QStyledItemDelegate* m_pNormalDelegate;
    QList<QByteArray> m_remainingCodecs;
  private slots:
    void slotFilepathButtonClicked();
    void slotMoreButtonComboboxPressed();
};
