#include <QTextCodec>
#include <QComboBox>
#include <QFileDialog>
#include <QDir>
#include <QErrorMessage>
#include "broadcast/listenersfinder.h"
#include "preferences/dialog/dlgprefmetadata.h"
#include "preferences/dialog/ui_dlgfilelistenerbox.h"

DlgPrefMetadata::DlgPrefMetadata(QWidget *pParent,UserSettingsPointer pSettings)
        : DlgPreferencePage(pParent),
          m_pSettings(pSettings),
          m_pFileSettings(nullptr) {
    setupUi(this);
    setFileSettings();
}

void DlgPrefMetadata::setFileSettings() {
    FileWidgets widgets;
    widgets.enableCheckbox = enableFileListener;
    widgets.enableCustomFormatBox = customFormatEnabledBox;
    widgets.encodingBox = fileEncodingComboBox;
    widgets.formatBox = formatComboBox;
    widgets.formatLineEdit = formatLineEdit;
    widgets.customFormatLineEdit = customFormatLineEdit;
    widgets.filePathLineEdit = filePathLineEdit;
    widgets.changeFilePathButton = filePathButton;

    m_pFileSettings = new MetadataFileSettings(m_pSettings,widgets,this);
}

void DlgPrefMetadata::slotApply() {
    m_pFileSettings->applySettings();
}

void DlgPrefMetadata::slotCancel() {
    m_pFileSettings->cancelSettings();
}

void DlgPrefMetadata::slotResetToDefaults() {
    m_pFileSettings->setSettingsToDefault();
}

DlgPrefMetadata::~DlgPrefMetadata() {
    delete m_pFileSettings;
}













