
#include "preferences/dialog/dlgprefmetadata.h"
#include "dlgprefmetadata.h"


DlgPrefMetadata::DlgPrefMetadata(QWidget *pParent,UserSettingsPointer pSettings)
        : DlgPreferencePage(pParent),
          m_pSettings(pSettings),
          m_pFileSettings(nullptr),
          m_pListenBrainzSettings(nullptr),
          m_pLastFMSettings(nullptr) {
    setupUi(this);
    setFileSettings();
    setListenBrainzSettings();
    setLastFMSettings();
}

void DlgPrefMetadata::setFileSettings() {
    FileWidgets widgets;
    widgets.enableCheckbox = enableFileListener;
    widgets.encodingBox = fileEncodingComboBox;
    widgets.formatLineEdit = fileFormatLineEdit;
    widgets.filePathLineEdit = filePathLineEdit;
    widgets.changeFilePathButton = filePathButton;

    m_pFileSettings = new MetadataFileSettings(m_pSettings,widgets,this);
}

void DlgPrefMetadata::setListenBrainzSettings() {
    ListenBrainzWidgets widgets;
    widgets.m_pEnabled = enableListenbrainzBox;
    widgets.m_pUserToken = listenBrainzUserTokenLineEdit;
    m_pListenBrainzSettings = new ListenBrainzSettingsManager(m_pSettings,widgets);
}

void DlgPrefMetadata::setLastFMSettings() {
    LastFMWidgets widgets;
    widgets.m_pEnabled = enableLastFmBox;
    m_pLastFMSettings = new LastFMSettingsManager(m_pSettings, widgets, this);
}

void DlgPrefMetadata::slotApply() {
    m_pFileSettings->applySettings();
    m_pListenBrainzSettings->applySettings();
    m_pLastFMSettings->applySettings();
}

void DlgPrefMetadata::slotCancel() {
    m_pFileSettings->cancelSettings();
    m_pListenBrainzSettings->cancelSettings();
    m_pLastFMSettings->cancelSettings();
}

void DlgPrefMetadata::slotResetToDefaults() {
    m_pFileSettings->setSettingsToDefault();
    m_pListenBrainzSettings->setSettingsToDefault();
    m_pLastFMSettings->setSettingsToDefault();
}

DlgPrefMetadata::~DlgPrefMetadata() {
    delete m_pFileSettings;
    delete m_pListenBrainzSettings;
    delete m_pLastFMSettings;
}















