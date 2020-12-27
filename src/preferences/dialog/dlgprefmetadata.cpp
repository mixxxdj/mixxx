#include "preferences/dialog/dlgprefmetadata.h"

#include "broadcast/scrobblingservice.h"
#include "moc_dlgprefmetadata.cpp"
#include "preferences/listenbrainzsettings.h"
#include "preferences/metadatafilesettings.h"

DlgPrefMetadata::DlgPrefMetadata(QWidget* pParent, const UserSettingsPointer& pSettings)
        : DlgPreferencePage(pParent),
          m_pSettings(pSettings),
          m_pFileSettings(nullptr),
          m_pListenBrainzSettings(nullptr) {
    setupUi(this);
    setFileSettings();
    setListenBrainzSettings();

    connect(enableListenbrainzBox,
            &QCheckBox::toggled,
            listenBrainzUserTokenLineEdit,
            &QLineEdit::setEnabled);
    connect(enableFileListener, &QCheckBox::toggled, fileFormatLineEdit, &QLineEdit::setEnabled);
    connect(enableFileListener, &QCheckBox::toggled, filePathButton, &QPushButton::setEnabled);
    connect(enableFileListener, &QCheckBox::toggled, filePathLineEdit, &QLineEdit::setEnabled);
    connect(enableFileListener, &QCheckBox::toggled, fileEncodingComboBox, &QComboBox::setEnabled);
}

void DlgPrefMetadata::setFileSettings() {
    FileWidgets widgets;
    widgets.enableCheckbox = enableFileListener;
    widgets.encodingBox = fileEncodingComboBox;
    widgets.formatLineEdit = fileFormatLineEdit;
    widgets.filePathLineEdit = filePathLineEdit;
    widgets.changeFilePathButton = filePathButton;

    m_pFileSettings = new MetadataFileSettings(m_pSettings, widgets, this);
}

void DlgPrefMetadata::setListenBrainzSettings() {
    ListenBrainzWidgets widgets;
    widgets.m_pEnabled = enableListenbrainzBox;
    widgets.m_pUserToken = listenBrainzUserTokenLineEdit;
    m_pListenBrainzSettings = new ListenBrainzSettingsManager(m_pSettings, widgets);
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
