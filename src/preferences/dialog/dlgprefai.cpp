#include "preferences/dialog/dlgprefai.h"

#include "moc_dlgprefai.cpp"

#include <QJsonArray>

#ifdef __QTKEYCHAIN__
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <qt6keychain/keychain.h>
#else
#include <qt5keychain/keychain.h>
#endif
#endif // __QTKEYCHAIN__

namespace {

using mixxx::ai::AiCandidateTrack;
using mixxx::ai::AiMixPlanner;
using mixxx::ai::AiMixSettings;

#ifdef __QTKEYCHAIN__
// The keychain entries are shared with mixxx::ai, so the names must match.
const QString kKeychainService = QStringLiteral("Mixxx - AI Assistant");

QString keychainKey(const QString& providerId) {
    return QStringLiteral("ApiKey-") + providerId;
}
#endif // __QTKEYCHAIN__

/// The combo box entries are added in the order of the Provider enum, so the
/// current index and the enum value are the same.
AiMixSettings::Provider providerAt(int index) {
    switch (index) {
    case static_cast<int>(AiMixSettings::Provider::OpenAI):
        return AiMixSettings::Provider::OpenAI;
    case static_cast<int>(AiMixSettings::Provider::Custom):
        return AiMixSettings::Provider::Custom;
    default:
        return AiMixSettings::Provider::DeepSeek;
    }
}

} // namespace

DlgPrefAi::DlgPrefAi(QWidget* pParent, UserSettingsPointer pConfig)
        : DlgPreferencePage(pParent),
          m_pConfig(pConfig),
          m_pPlanner(new AiMixPlanner(this, pConfig)) {
    setupUi(this);

    for (const auto provider : {AiMixSettings::Provider::DeepSeek,
                 AiMixSettings::Provider::OpenAI,
                 AiMixSettings::Provider::Custom}) {
        AiProviderComboBox->addItem(AiMixSettings::providerDisplayName(provider));
    }

    if (mixxx::ai::isApiKeyStorageAvailable()) {
        AiKeychainWarningLabel->hide();
    } else {
        AiKeychainWarningLabel->show();
        AiApiKeyLineEdit->setEnabled(false);
    }

    connect(AiProviderComboBox,
            &QComboBox::currentIndexChanged,
            this,
            &DlgPrefAi::slotProviderChanged);
    connect(AiTestButton, &QPushButton::clicked, this, &DlgPrefAi::slotTestConnection);
    connect(m_pPlanner, &AiMixPlanner::succeeded, this, &DlgPrefAi::slotProbeSucceeded);
    connect(m_pPlanner, &AiMixPlanner::failed, this, &DlgPrefAi::slotProbeFailed);

    setScrollSafeGuardForAllInputWidgets(this);

    // The values are filled in by slotUpdate(), which the preferences dialog
    // calls when this page is shown.
}

DlgPrefAi::~DlgPrefAi() = default;

void DlgPrefAi::setStatusMessage(const QString& message, bool isError) {
    AiStatusLabel->setText(message);
    AiStatusLabel->setStyleSheet(isError ? QStringLiteral("color: #d9534f;")
                                         : QString());
}

void DlgPrefAi::loadProviderDefaults(AiMixSettings::Provider provider) {
    AiBaseUrlLineEdit->setText(AiMixSettings::defaultBaseUrl(provider));
    AiModelLineEdit->setText(AiMixSettings::defaultModel(provider));
    AiApiKeyLineEdit->clear();
    requestStoredApiKey(AiMixSettings::providerId(provider));
}

void DlgPrefAi::slotProviderChanged(int index) {
    if (m_loading) {
        return;
    }
    // Switching the provider also switches the keychain entry, so the fields
    // are reloaded instead of keeping values that belong to another service.
    loadProviderDefaults(providerAt(index));
    setStatusMessage(QString(), false);
}

void DlgPrefAi::slotUpdate() {
    const AiMixSettings settings = AiMixSettings::fromUserSettings(*m_pConfig);

    m_loading = true;
    AiEnabledCheckBox->setChecked(settings.enabled);
    AiProviderComboBox->setCurrentIndex(static_cast<int>(settings.provider));
    AiBaseUrlLineEdit->setText(settings.baseUrl);
    AiModelLineEdit->setText(settings.model);
    AiApiKeyLineEdit->clear();
    requestStoredApiKey(AiMixSettings::providerId(settings.provider));
    AiMaxTracksSpinBox->setValue(settings.maxCandidateTracks);
    AiTimeoutSpinBox->setValue(settings.timeoutSeconds);
    m_loading = false;

    setStatusMessage(QString(), false);
}

void DlgPrefAi::slotApply() {
    AiMixSettings settings = AiMixSettings::fromUserSettings(*m_pConfig);
    settings.provider = providerAt(AiProviderComboBox->currentIndex());
    settings.baseUrl = AiBaseUrlLineEdit->text().trimmed();
    settings.model = AiModelLineEdit->text().trimmed();
    settings.maxCandidateTracks = AiMaxTracksSpinBox->value();
    settings.timeoutSeconds = AiTimeoutSpinBox->value();
    settings.enabled = AiEnabledCheckBox->isChecked();
    settings.writeToUserSettings(m_pConfig.data());

    if (!mixxx::ai::isApiKeyStorageAvailable()) {
        return;
    }
    // An empty field means "no key": the keychain entry is removed then.
    writeStoredApiKey(AiMixSettings::providerId(settings.provider),
            AiApiKeyLineEdit->text().trimmed());
}

void DlgPrefAi::slotResetToDefaults() {
    m_loading = true;
    AiEnabledCheckBox->setChecked(false);
    AiProviderComboBox->setCurrentIndex(static_cast<int>(AiMixSettings::Provider::DeepSeek));
    AiMaxTracksSpinBox->setValue(AiMixSettings::kDefaultMaxCandidateTracks);
    AiTimeoutSpinBox->setValue(AiMixSettings::kDefaultTimeoutSeconds);
    loadProviderDefaults(AiMixSettings::Provider::DeepSeek);
    m_loading = false;

    setStatusMessage(QString(), false);
}

void DlgPrefAi::slotTestConnection() {
    if (m_pPlanner->isBusy()) {
        return;
    }

    // The probe runs with the values currently on this page, so they have to be
    // stored first: that is what the button label promises.
    slotApply();

    AiCandidateTrack probeTrack;
    probeTrack.index = 0;
    probeTrack.title = QStringLiteral("Connection test");
    QJsonArray candidates;
    candidates.append(probeTrack.toJson());

    QString error;
    m_testingConnection = true;
    if (!m_pPlanner->requestSetList(
                QStringLiteral("Select the only track to verify the connection."),
                candidates,
                &error)) {
        m_testingConnection = false;
        setStatusMessage(error, true);
        return;
    }

    AiTestButton->setEnabled(false);
    setStatusMessage(tr("Contacting the service..."), false);
}

void DlgPrefAi::slotProbeSucceeded(const QList<int>& trackOrder) {
    Q_UNUSED(trackOrder);
    if (!m_testingConnection) {
        return;
    }
    m_testingConnection = false;
    AiTestButton->setEnabled(true);
    setStatusMessage(tr("The connection works, the service answered correctly."), false);
}

void DlgPrefAi::slotProbeFailed(const QString& errorMessage) {
    if (!m_testingConnection) {
        return;
    }
    m_testingConnection = false;
    AiTestButton->setEnabled(true);
    setStatusMessage(errorMessage, true);
}

void DlgPrefAi::requestStoredApiKey(const QString& providerId) {
#ifdef __QTKEYCHAIN__
    auto* pJob = new QKeychain::ReadPasswordJob(kKeychainService, this);
    pJob->setAutoDelete(true);
    pJob->setKey(keychainKey(providerId));
    connect(pJob, &QKeychain::ReadPasswordJob::finished, this, [this, pJob]() {
        if (pJob->error() == QKeychain::NoError) {
            AiApiKeyLineEdit->setText(pJob->textData());
        }
    });
    pJob->start();
#else
    Q_UNUSED(providerId);
#endif // __QTKEYCHAIN__
}

void DlgPrefAi::writeStoredApiKey(const QString& providerId, const QString& apiKey) {
#ifdef __QTKEYCHAIN__
    if (apiKey.isEmpty()) {
        auto* pJob = new QKeychain::DeletePasswordJob(kKeychainService, this);
        pJob->setAutoDelete(true);
        pJob->setKey(keychainKey(providerId));
        connect(pJob, &QKeychain::DeletePasswordJob::finished, this, [this, pJob]() {
            if (pJob->error() != QKeychain::NoError &&
                    pJob->error() != QKeychain::EntryNotFound) {
                setStatusMessage(pJob->errorString(), true);
            }
        });
        pJob->start();
        return;
    }

    auto* pJob = new QKeychain::WritePasswordJob(kKeychainService, this);
    pJob->setAutoDelete(true);
    pJob->setKey(keychainKey(providerId));
    pJob->setTextData(apiKey);
    connect(pJob, &QKeychain::WritePasswordJob::finished, this, [this, pJob]() {
        if (pJob->error() != QKeychain::NoError) {
            setStatusMessage(pJob->errorString(), true);
        }
    });
    pJob->start();
#else
    Q_UNUSED(providerId);
    Q_UNUSED(apiKey);
#endif // __QTKEYCHAIN__
}