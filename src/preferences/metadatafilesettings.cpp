#include "metadatafilesettings.h"

#include <QFileDialog>
#include <QLineEdit>
#include <QObject>
#include <QTextCodec>

#include "moc_metadatafilesettings.cpp"

FileSettings MetadataFileSettings::s_latestSettings;

MetadataFileSettings::MetadataFileSettings(UserSettingsPointer pSettings,
        const FileWidgets& widgets,
        QWidget* dialogWidget)
        : m_pSettings(pSettings),
          m_CPSettingsChanged(kSettingsChanged),
          m_widgets(widgets),
          m_pDialogWidget(dialogWidget) {
    s_latestSettings = getPersistedSettings(pSettings);
    setupWidgets();
}

FileSettings MetadataFileSettings::getPersistedSettings(const UserSettingsPointer& pSettings) {
    FileSettings ret;
    ret.enabled =
            pSettings->getValue(kMetadataFileEnabled, defaultFileMetadataEnabled);
    ret.fileEncoding =
            pSettings->getValue(kFileEncoding, defaultEncoding.constData()).toUtf8();
    ret.fileFormat =
            pSettings->getValue(kFileFormat, defaultFileFormat);
    ret.fileFormatString =
            pSettings->getValue(kFileFormatString, defaultFileFormatString);
    ret.filePath =
            pSettings->getValue(kFilePath, defaultFilePath);
    return ret;
}

void MetadataFileSettings::setupWidgets() {
    m_widgets.enableCheckbox->setChecked(s_latestSettings.enabled);

    m_widgets.encodingBox->clear();
    QList<QByteArray> codecs = QTextCodec::availableCodecs();
    for (const QByteArray& codec : codecs) {
        m_widgets.encodingBox->addItem(codec);
    }

    m_widgets.formatBox->clear();
    //To be extended when adding more file formats.
    QVariant SAMBroadcasterData("author - title");
    m_widgets.formatBox->addItem("SAMBroadcaster", SAMBroadcasterData);

    m_widgets.formatLineEdit->setText(m_widgets.formatBox->itemData(
                                                                 m_widgets.formatBox->currentIndex())
                                              .toString());
    QObject::connect(m_widgets.formatBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotFormatChanged(int)));

    m_widgets.enableCustomFormatBox->setChecked(s_latestSettings.fileFormat == "Custom");
    if (s_latestSettings.fileFormat == "Custom") {
        m_widgets.customFormatLineEdit->setText(s_latestSettings.fileFormatString);
    }

    m_widgets.filePathLineEdit->setText(s_latestSettings.filePath);
    m_widgets.filePathLineEdit->setStyleSheet("");
    QObject::connect(m_widgets.changeFilePathButton, SIGNAL(pressed()), this, SLOT(slotFilepathButtonClicked()));
}

FileSettings MetadataFileSettings::getLatestSettings() {
    return MetadataFileSettings::s_latestSettings;
}

void MetadataFileSettings::applySettings() {
    if (fileSettingsDifferent() && checkIfSettingsCorrect()) {
        saveLatestSettingsAndNotify();
        persistSettings();
    }
}

bool MetadataFileSettings::fileSettingsDifferent() {
    return s_latestSettings.enabled !=
            m_widgets.enableCheckbox->isChecked() ||

            s_latestSettings.fileEncoding !=
            m_widgets.encodingBox->currentText() ||

            s_latestSettings.fileFormat != "Custom" &&
            s_latestSettings.fileFormat !=
                    m_widgets.formatBox->currentText() ||

            s_latestSettings.fileFormat != "Custom" &&
            m_widgets.enableCustomFormatBox->isChecked() ||

            s_latestSettings.fileFormat == "Custom" &&
            !m_widgets.enableCustomFormatBox->isChecked() ||

            s_latestSettings.fileFormat == "Custom" &&
            s_latestSettings.fileFormatString !=
                    m_widgets.customFormatLineEdit->text() ||

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

void MetadataFileSettings::saveLatestSettingsAndNotify() {
    FileSettings ret;
    ret.enabled = m_widgets.enableCheckbox->isChecked();
    ret.fileEncoding = m_widgets.encodingBox->currentText().toUtf8();
    ret.fileFormat =
            m_widgets.enableCustomFormatBox->isChecked() ? "Custom" : m_widgets.formatBox->currentText();
    ret.fileFormatString =
            m_widgets.enableCustomFormatBox->isChecked() ? m_widgets.customFormatLineEdit->text() : m_widgets.formatLineEdit->text();
    ret.filePath = QDir(m_widgets.filePathLineEdit->text()).absolutePath();
    s_latestSettings = ret;
    m_CPSettingsChanged.set(true);
}

void MetadataFileSettings::persistSettings() {
    m_pSettings->setValue(kMetadataFileEnabled, s_latestSettings.enabled);
    m_pSettings->setValue(kFileEncoding, QString(s_latestSettings.fileEncoding));
    m_pSettings->setValue(kFileFormat, s_latestSettings.fileFormat);
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
    s_latestSettings.fileFormat = defaultFileFormat;
    s_latestSettings.fileFormatString = defaultFileFormatString;
    s_latestSettings.filePath = defaultFilePath;
}

void MetadataFileSettings::slotFormatChanged(int newIndex) {
    m_widgets.formatLineEdit->setText(m_widgets.formatBox->itemData(newIndex).toString());
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
