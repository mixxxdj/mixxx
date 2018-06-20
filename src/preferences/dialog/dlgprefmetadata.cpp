#include "preferences/dialog/dlgprefmetadata.h"

#include <QComboBox>
#include <QDir>
#include <QErrorMessage>
#include <QFileDialog>
#include <QTextCodec>

#include "broadcast/listenersfinder.h"
#include "moc_dlgprefmetadata.cpp"
#include "preferences/dialog/ui_dlgfilelistenerbox.h"

DlgPrefMetadata::DlgPrefMetadata(QWidget* pParent, UserSettingsPointer pSettings)
        : DlgPreferencePage(pParent),
          m_pSettings(pSettings),
          m_CPSettingsChanged(kSettingsChanged) {
    setupUi(this);
    getPersistedSettings(pSettings);
    setupWidgets();
}

void DlgPrefMetadata::getPersistedSettings(UserSettingsPointer pSettings) {
    m_latestSettings.enabled =
            pSettings->getValue(kMetadataFileEnabled, defaultFileMetadataEnabled);
    m_latestSettings.fileEncoding =
            pSettings->getValue(kFileEncoding, defaultEncoding);
    m_latestSettings.fileFormat =
            pSettings->getValue(kFileFormat, defaultFileFormat);
    m_latestSettings.fileFormatString =
            pSettings->getValue(kFileFormatString, defaultFileFormatString);
    m_latestSettings.filePath =
            pSettings->getValue(kFilePath, defaultFilePath);
}

void DlgPrefMetadata::setupWidgets() {
    enableFileListener->setChecked(m_latestSettings.enabled);

    fileEncodingComboBox->clear();
    QList<QByteArray> codecs = QTextCodec::availableCodecs();
    for (const QByteArray& codec : codecs) {
        fileEncodingComboBox->addItem(codec);
    }

    formatComboBox->clear();
    //To be extended when adding more file formats.
    QVariant SAMBroadcasterData("author - title");
    formatComboBox->addItem("SAMBroadcaster", SAMBroadcasterData);

    formatLineEdit->setText(formatComboBox->itemData(formatComboBox->currentIndex()).toString());
    connect(formatComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotFormatChanged(int)));

    customFormatEnabledBox->setChecked(m_latestSettings.fileFormat == "Custom");
    if (m_latestSettings.fileFormat == "Custom")
        customFormatLineEdit->setText(m_latestSettings.fileFormatString);

    filePathLineEdit->setText(m_latestSettings.filePath);
    filePathLineEdit->setStyleSheet("");
    connect(filePathButton, SIGNAL(pressed()), this, SLOT(slotFilepathButtonClicked()));
}

void DlgPrefMetadata::slotFormatChanged(int newIndex) {
    formatLineEdit->setText(formatComboBox->itemData(newIndex).toString());
}

void DlgPrefMetadata::slotFilepathButtonClicked() {
    QString newFilePath = QFileDialog::getSaveFileName(
            this,
            "Choose new file path",
            "./",
            "Text files(*.txt)");
    filePathLineEdit->setText(newFilePath);
}

void DlgPrefMetadata::slotApply() {
    if (fileSettingsDifferent() && checkIfSettingsCorrect()) {
        saveLatestSettingsAndNotify();
        persistSettings();
    }
}

bool DlgPrefMetadata::fileSettingsDifferent() {
    return (m_latestSettings.enabled != enableFileListener->isChecked() ||
            m_latestSettings.fileEncoding != fileEncodingComboBox->currentText() ||
            (m_latestSettings.fileFormat != "Custom" && m_latestSettings.fileFormat != formatComboBox->currentText()) ||
            (m_latestSettings.fileFormat != "Custom" && customFormatEnabledBox->isChecked()) ||
            (m_latestSettings.fileFormat == "Custom" && !customFormatEnabledBox->isChecked()) ||
            (m_latestSettings.fileFormat == "Custom" && m_latestSettings.fileFormatString != customFormatLineEdit->text()) ||
            m_latestSettings.filePath != filePathLineEdit->text());
}

bool DlgPrefMetadata::checkIfSettingsCorrect() {
    QString supposedPath = filePathLineEdit->text();
    int lastIndex = supposedPath.lastIndexOf('/');
    if (lastIndex != -1) {
        QString supposedDir = supposedPath.left(lastIndex);
        QDir dir(supposedDir);
        bool dirExists = dir.exists();
        if (!dirExists) {
            filePathLineEdit->setStyleSheet("border: 1px solid red");
        } else {
            filePathLineEdit->setStyleSheet("");
        }
        return dirExists;
    }
    return true;
}

void DlgPrefMetadata::saveLatestSettingsAndNotify() {
    m_latestSettings.enabled = enableFileListener->isChecked();
    m_latestSettings.fileEncoding = fileEncodingComboBox->currentText();
    m_latestSettings.fileFormat =
            customFormatEnabledBox->isChecked() ? "Custom" : formatComboBox->currentText();
    m_latestSettings.fileFormatString =
            customFormatEnabledBox->isChecked() ? customFormatLineEdit->text() : formatLineEdit->text();
    m_latestSettings.filePath = QDir(filePathLineEdit->text()).absolutePath();
    m_CPSettingsChanged.set(true);
}

void DlgPrefMetadata::persistSettings() {
    m_pSettings->setValue(kMetadataFileEnabled, m_latestSettings.enabled);
    m_pSettings->setValue(kFileEncoding, m_latestSettings.fileEncoding);
    m_pSettings->setValue(kFileFormat, m_latestSettings.fileFormat);
    m_pSettings->setValue(kFileFormatString, m_latestSettings.fileFormatString);
    m_pSettings->setValue(kFilePath, m_latestSettings.filePath);
}

void DlgPrefMetadata::slotCancel() {
    setupWidgets();
}

void DlgPrefMetadata::slotResetToDefaults() {
    resetSettingsToDefault();
    setupWidgets();
}

void DlgPrefMetadata::resetSettingsToDefault() {
    m_latestSettings.enabled = defaultFileMetadataEnabled;
    m_latestSettings.fileEncoding = defaultEncoding;
    m_latestSettings.fileFormat = defaultFileFormat;
    m_latestSettings.fileFormatString = defaultFileFormatString;
    m_latestSettings.filePath = defaultFilePath;
}

FileSettings DlgPrefMetadata::getLatestSettings() {
    return m_latestSettings;
}

void DlgPrefMetadata::slotUpdate() {
}
