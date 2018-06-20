#include "preferences/dialog/dlgprefmetadata.h"

#include <QComboBox>
#include <QDir>
#include <QErrorMessage>
#include <QFileDialog>
#include <QTextCodec>

#include "broadcast/listenersfinder.h"
#include "moc_dlgprefmetadata.cpp"
#include "preferences/dialog/ui_dlgfilelistenerbox.h"

FileSettings DlgPrefMetadata::s_latestSettings;

DlgPrefMetadata::DlgPrefMetadata(QWidget *pParent,UserSettingsPointer pSettings)
        : DlgPreferencePage(pParent),
          m_pSettings(pSettings),
          m_CPSettingsChanged(kSettingsChanged) {
    setupUi(this);
    s_latestSettings = getPersistedSettings(pSettings);
    setupWidgets();
}

FileSettings DlgPrefMetadata::getPersistedSettings(const UserSettingsPointer &pSettings) {
    FileSettings ret;
    ret.enabled =
            pSettings->getValue(kMetadataFileEnabled,defaultFileMetadataEnabled);
    ret.fileEncoding =
            pSettings->getValue(kFileEncoding,defaultEncoding.constData()).toUtf8();
    ret.fileFormat =
            pSettings->getValue(kFileFormat,defaultFileFormat);
    ret.fileFormatString =
            pSettings->getValue(kFileFormatString,defaultFileFormatString);
    ret.filePath =
            pSettings->getValue(kFilePath,defaultFilePath);
    return ret;
}

void DlgPrefMetadata::setupWidgets() {
    enableFileListener->setChecked(s_latestSettings.enabled);

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

    customFormatEnabledBox->setChecked(s_latestSettings.fileFormat == "Custom");
    if (s_latestSettings.fileFormat == "Custom")
        customFormatLineEdit->setText(s_latestSettings.fileFormatString);

    filePathLineEdit->setText(s_latestSettings.filePath);
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
    return (s_latestSettings.enabled != enableFileListener->isChecked() ||
            s_latestSettings.fileEncoding !=  fileEncodingComboBox->currentText() ||
            (s_latestSettings.fileFormat != "Custom" && s_latestSettings.fileFormat != formatComboBox->currentText()) ||
            (s_latestSettings.fileFormat != "Custom" && customFormatEnabledBox->isChecked()) ||
            (s_latestSettings.fileFormat == "Custom" && !customFormatEnabledBox->isChecked()) ||
            (s_latestSettings.fileFormat == "Custom" && s_latestSettings.fileFormatString != customFormatLineEdit->text()) ||
            s_latestSettings.filePath != filePathLineEdit->text());
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
    s_latestSettings.enabled = enableFileListener->isChecked();
    s_latestSettings.fileEncoding = fileEncodingComboBox->currentText().toUtf8();
    s_latestSettings.fileFormat =
            customFormatEnabledBox->isChecked() ? "Custom" :
            formatComboBox->currentText();
    s_latestSettings.fileFormatString =
            customFormatEnabledBox->isChecked() ? customFormatLineEdit->text() :
            formatLineEdit->text();
    s_latestSettings.filePath = QDir(filePathLineEdit->text()).absolutePath();
    m_CPSettingsChanged.set(true);
}

void DlgPrefMetadata::persistSettings() {
    m_pSettings->setValue(kMetadataFileEnabled,s_latestSettings.enabled);
    m_pSettings->setValue(kFileEncoding,QString(s_latestSettings.fileEncoding));
    m_pSettings->setValue(kFileFormat,s_latestSettings.fileFormat);
    m_pSettings->setValue(kFileFormatString,s_latestSettings.fileFormatString);
    m_pSettings->setValue(kFilePath,s_latestSettings.filePath);
}

void DlgPrefMetadata::slotCancel() {
    setupWidgets();
}

void DlgPrefMetadata::slotResetToDefaults() {
    resetSettingsToDefault();
    setupWidgets();
}

void DlgPrefMetadata::resetSettingsToDefault() {
    s_latestSettings.enabled = defaultFileMetadataEnabled;
    s_latestSettings.fileEncoding = defaultEncoding;
    s_latestSettings.fileFormat = defaultFileFormat;
    s_latestSettings.fileFormatString = defaultFileFormatString;
    s_latestSettings.filePath = defaultFilePath;
}

FileSettings DlgPrefMetadata::getLatestSettings() {
    return DlgPrefMetadata::s_latestSettings;
}

void DlgPrefMetadata::slotUpdate() {
}
