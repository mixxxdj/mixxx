#include "preferences/metadatafilesettings.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QFileDialog>
#include <QLineEdit>
#include <QMouseEvent>
#include <QObject>
#include <QTextCodec>
#include <algorithm>

#include "moc_metadatafilesettings.cpp"

FileSettings MetadataFileSettings::s_latestSettings;

MetadataFileSettings::MetadataFileSettings(UserSettingsPointer pSettings,
        const FileWidgets& widgets,
        QWidget* dialogWidget)
        : m_pSettings(pSettings),
          m_CPSettingsChanged(kFileSettingsChanged),
          m_widgets(widgets),
          m_pDialogWidget(dialogWidget),
          m_fileEncodings{
                  "UTF-8",
                  "latin1",
                  "Windows-1251",
                  "Windows-1252",
                  "Shift-JIS",
                  "GB18030",
                  "EUC-KR",
                  "EUC-JP"} {
    s_latestSettings = getPersistedSettings(pSettings);
    setupWidgets();
}

FileSettings MetadataFileSettings::getPersistedSettings(const UserSettingsPointer& pSettings) {
    FileSettings ret;
    ret.enabled =
            pSettings->getValue(kMetadataFileEnabled, defaultFileMetadataEnabled);
    ret.fileEncoding =
            pSettings->getValue(kFileEncoding, defaultEncoding.constData()).toUtf8();
    ret.fileFormatString =
            pSettings->getValue(kFileFormatString, defaultFileFormatString);
    ret.filePath =
            pSettings->getValue(kFilePath, defaultFilePath);
    return ret;
}

void MetadataFileSettings::setupWidgets() {
    m_widgets.enableCheckbox->setChecked(s_latestSettings.enabled);

    setupEncodingComboBox();

    m_widgets.formatLineEdit->setText(s_latestSettings.fileFormatString);

    m_widgets.filePathLineEdit->setText(s_latestSettings.filePath);
    m_widgets.filePathLineEdit->setStyleSheet("");
    QObject::connect(m_widgets.changeFilePathButton,
            SIGNAL(pressed()),
            this,
            SLOT(slotFilepathButtonClicked()));
}

FileSettings MetadataFileSettings::getLatestSettings() {
    return MetadataFileSettings::s_latestSettings;
}

void MetadataFileSettings::applySettings() {
    if (fileSettingsDifferent() && checkIfSettingsCorrect()) {
        updateLatestSettingsAndNotify();
        persistSettings();
    }
}

bool MetadataFileSettings::fileSettingsDifferent() {
    return s_latestSettings.enabled !=
            m_widgets.enableCheckbox->isChecked() ||

            s_latestSettings.fileEncoding !=
            m_widgets.encodingBox->currentText() ||

            s_latestSettings.fileFormatString !=
            m_widgets.formatLineEdit->text() ||

            s_latestSettings.filePath != m_widgets.filePathLineEdit->text();
}

bool MetadataFileSettings::checkIfSettingsCorrect() {
    QString supposedPath = m_widgets.filePathLineEdit->text();
    int lastIndex = supposedPath.lastIndexOf('/');
    if (lastIndex != -1) {
        QString supposedDir = supposedPath.left(lastIndex);
        QDir dir(supposedDir);
        bool dirExists = dir.exists();
        if (!dirExists) {
            m_widgets.filePathLineEdit->setStyleSheet("border: 1px solid red");
        } else {
            m_widgets.filePathLineEdit->setStyleSheet("");
        }
        return dirExists;
    }
    return true;
}

void MetadataFileSettings::updateLatestSettingsAndNotify() {
    FileSettings ret;
    ret.enabled = m_widgets.enableCheckbox->isChecked();
    ret.fileEncoding = m_widgets.encodingBox->currentText().toUtf8();
    ret.fileFormatString = m_widgets.formatLineEdit->text();
    ret.filePath = QDir(m_widgets.filePathLineEdit->text()).absolutePath();
    s_latestSettings = ret;
    m_CPSettingsChanged.set(true);
}

void MetadataFileSettings::persistSettings() {
    m_pSettings->setValue(kMetadataFileEnabled, s_latestSettings.enabled);
    m_pSettings->setValue(kFileEncoding, QString(s_latestSettings.fileEncoding));
    m_pSettings->setValue(kFileFormatString, s_latestSettings.fileFormatString);
    m_pSettings->setValue(kFilePath, s_latestSettings.filePath);
}

void MetadataFileSettings::setSettingsToDefault() {
    resetSettingsToDefault();
    setupWidgets();
}

void MetadataFileSettings::resetSettingsToDefault() {
    s_latestSettings.enabled = defaultFileMetadataEnabled;
    s_latestSettings.fileEncoding = defaultEncoding;
    s_latestSettings.fileFormatString = defaultFileFormatString;
    s_latestSettings.filePath = defaultFilePath;
}

void MetadataFileSettings::slotFilepathButtonClicked() {
    QString newFilePath = QFileDialog::getSaveFileName(
            m_pDialogWidget,
            "Choose new file path",
            checkIfSettingsCorrect() ? m_widgets.filePathLineEdit->text() : defaultFilePath,
            "Text files(*.txt)");
    m_widgets.filePathLineEdit->setText(newFilePath);
}

void MetadataFileSettings::cancelSettings() {
    setupWidgets();
}

void MetadataFileSettings::setupEncodingComboBox() {
    m_widgets.encodingBox->clear();

    for (const QByteArray& fileEncoding : m_fileEncodings) {
        DEBUG_ASSERT(QTextCodec::codecForName(fileEncoding) != nullptr);
        m_widgets.encodingBox->addItem(fileEncoding);
    }

    if (!m_fileEncodings.contains(QTextCodec::codecForLocale()->name())) {
        m_widgets.encodingBox->addItem(QTextCodec::codecForLocale()->name());
        DEBUG_ASSERT(QTextCodec::codecForName(QTextCodec::codecForLocale()->name()) != nullptr);
    }

    m_widgets.encodingBox->setCurrentText(s_latestSettings.fileEncoding);
}
