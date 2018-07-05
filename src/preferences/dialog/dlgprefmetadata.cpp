#include "preferences/dialog/dlgprefmetadata.h"

#include "moc_dlgprefmetadata.cpp"

DlgPrefMetadata::DlgPrefMetadata(QWidget* pParent, UserSettingsPointer pSettings)
        : DlgPreferencePage(pParent),
          m_pSettings(pSettings),
          m_pFileSettings(nullptr),
          m_pListenBrainzSettings(nullptr) {
    setupUi(this);
    setFileSettings();
    setListenBrainzSettings();
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

    m_pFileSettings = new MetadataFileSettings(m_pSettings, widgets, this);
}

void DlgPrefMetadata::setListenBrainzSettings() {
    ListenBrainzWidgets widgets;
    widgets.m_pEnabled = enableListenbrainzBox;
    widgets.m_pUserToken = listenBrainzUserTokenLineEdit;
    m_pListenBrainzSettings = new ListenBrainzSettingsManager(m_pSettings,widgets);
}

void DlgPrefMetadata::slotApply() {
    m_pFileSettings->applySettings();
    m_pListenBrainzSettings->applySettings();
}

void DlgPrefMetadata::slotCancel() {
    m_pFileSettings->cancelSettings();
    m_pListenBrainzSettings->cancelSettings();
}

void DlgPrefMetadata::slotResetToDefaults() {
    m_pFileSettings->setSettingsToDefault();
    m_pListenBrainzSettings->setSettingsToDefault();
}

DlgPrefMetadata::~DlgPrefMetadata() {
    delete m_pFileSettings;
    delete m_pListenBrainzSettings;
}

void DlgPrefMetadata::slotUpdate() {
}
