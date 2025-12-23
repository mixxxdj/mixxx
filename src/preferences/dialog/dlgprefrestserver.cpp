#include "preferences/dialog/dlgprefrestserver.h"

#include <QAbstractItemView>
#include <QClipboard>
#include <QDateTime>
#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QHeaderView>
#include <QIcon>
#include <QLocale>
#include <QSignalBlocker>
#include <QSslCertificate>
#include <QTableWidgetItem>
#include <QTimeZone>
#include <QUrl>
#include <limits>
#include <QUuid>
#include <QtGlobal>

#include "moc_dlgprefrestserver.cpp"
#include "network/rest/restscopes.h"

namespace {
constexpr int kCertificateExpiryWarningDays = 30;
constexpr int kPresetLocal = 0;
constexpr int kPresetLocalHttps = 1;
constexpr int kPresetNetwork = 2;
constexpr int kExpirationPresetNever = 0;
constexpr int kExpirationPresetOneMonth = 1;
constexpr int kExpirationPresetThreeMonths = 2;
constexpr int kExpirationPresetOneYear = 3;
constexpr int kExpirationPresetCustom = 4;
} // namespace

DlgPrefRestServer::DlgPrefRestServer(QWidget* parent, std::shared_ptr<RestServerSettings> settings)
        : DlgPreferencePage(parent),
          m_settings(std::move(settings)) {
    setupUi(this);
    createLinkColor();
#ifndef MIXXX_HAS_HTTP_SERVER
    labelRestApiUnavailable->setVisible(true);
    checkBoxEnableRestServer->setEnabled(false);
    groupBoxNetwork->setEnabled(false);
    groupBoxAuthentication->setEnabled(false);
    groupBoxTls->setEnabled(false);
#endif

    spinBoxHttpPort->setMinimum(1);
    spinBoxHttpPort->setMaximum(std::numeric_limits<quint16>::max());
    spinBoxHttpsPort->setMinimum(1);
    spinBoxHttpsPort->setMaximum(std::numeric_limits<quint16>::max());
    spinBoxMaxRequestSize->setMinimum(1);
    spinBoxMaxRequestSize->setMaximum(std::numeric_limits<int>::max() / 1024);

    labelAuthWarningIcon->setPixmap(QIcon(kWarningIconPath).pixmap(20, 20));
    labelUnauthWarningIcon->setPixmap(QIcon(kWarningIconPath).pixmap(20, 20));
    labelNetworkWarningIcon->setPixmap(QIcon(kWarningIconPath).pixmap(20, 20));
    labelStatusIcon->setPixmap(QIcon(kWarningIconPath).pixmap(20, 20));
    labelTlsStatusIcon->setPixmap(QIcon(kWarningIconPath).pixmap(20, 20));
    labelTlsCertificateIcon->setPixmap(QIcon(kWarningIconPath).pixmap(20, 20));

    tableTokens->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableTokens->setSelectionMode(QAbstractItemView::SingleSelection);
    tableTokens->horizontalHeader()->setStretchLastSection(true);
    dateTimeEditTokenExpires->setMinimumDateTime(
            QDateTime(QDate(1970, 1, 1), QTime(0, 0), QTimeZone::utc()));
    dateTimeEditTokenExpires->setTimeZone(QTimeZone::utc());
    dateTimeEditTokenExpires->setEnabled(false);
    lineEditTokenValue->setEchoMode(QLineEdit::Password);
    pushButtonToggleToken->setText(tr("Show"));

    comboBoxPreset->addItem(tr("Local (recommended)"));
    comboBoxPreset->addItem(tr("Local + HTTPS"));
    comboBoxPreset->addItem(tr("Network (advanced)"));
    comboBoxTokenExpiresPreset->addItem(tr("Never"));
    comboBoxTokenExpiresPreset->addItem(tr("+1 month"));
    comboBoxTokenExpiresPreset->addItem(tr("+3 months"));
    comboBoxTokenExpiresPreset->addItem(tr("+1 year"));
    comboBoxTokenExpiresPreset->addItem(tr("Custom"));

    connect(comboBoxPreset,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefRestServer::slotPresetChanged);
    connect(checkBoxEnableHttp,
            &QCheckBox::toggled,
            this,
            &DlgPrefRestServer::slotEnableHttpChanged);
    connect(checkBoxUseHttps,
            &QCheckBox::toggled,
            this,
            &DlgPrefRestServer::slotUseHttpsChanged);
    connect(checkBoxAutoGenerateCertificate,
            &QCheckBox::toggled,
            this,
            &DlgPrefRestServer::slotAutoGenerateCertificateChanged);
    connect(checkBoxEnableRestServer,
            &QCheckBox::toggled,
            this,
            &DlgPrefRestServer::slotEnableRestServerChanged);
    connect(checkBoxAllowUnauthenticated,
            &QCheckBox::toggled,
            this,
            [this] {
                updateUnauthWarning();
            });
    connect(lineEditCorsAllowlist,
            &QLineEdit::textChanged,
            this,
            &DlgPrefRestServer::slotCorsAllowlistChanged);
    connect(lineEditHost,
            &QLineEdit::textChanged,
            this,
            [this] {
                updateUrlLabels();
            });
    connect(spinBoxHttpPort,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            [this] {
                updateUrlLabels();
            });
    connect(spinBoxHttpsPort,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            [this] {
                updateUrlLabels();
            });
    connect(lineEditCertPath,
            &QLineEdit::textChanged,
            this,
            [this] {
                updateTlsCertificateStatus();
            });
    connect(pushButtonBrowseCert,
            &QPushButton::clicked,
            this,
            &DlgPrefRestServer::slotBrowseCertificate);
    connect(pushButtonBrowseKey,
            &QPushButton::clicked,
            this,
            &DlgPrefRestServer::slotBrowseKey);
    connect(pushButtonAddToken,
            &QPushButton::clicked,
            this,
            &DlgPrefRestServer::slotAddToken);
    connect(pushButtonRemoveToken,
            &QPushButton::clicked,
            this,
            &DlgPrefRestServer::slotRemoveToken);
    connect(pushButtonRegenerateToken,
            &QPushButton::clicked,
            this,
            &DlgPrefRestServer::slotRegenerateToken);
    connect(pushButtonCopyToken,
            &QPushButton::clicked,
            this,
            &DlgPrefRestServer::slotCopyToken);
    connect(pushButtonToggleToken,
            &QPushButton::clicked,
            this,
            &DlgPrefRestServer::slotToggleTokenVisibility);
    connect(pushButtonCopyHttpUrl,
            &QPushButton::clicked,
            this,
            [this] {
                QGuiApplication::clipboard()->setText(makeHttpUrl());
            });
    connect(pushButtonCopyHttpsUrl,
            &QPushButton::clicked,
            this,
            [this] {
                QGuiApplication::clipboard()->setText(makeHttpsUrl());
            });
    connect(tableTokens->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &DlgPrefRestServer::slotTokenSelectionChanged);
    connect(lineEditTokenDescription,
            &QLineEdit::textChanged,
            this,
            &DlgPrefRestServer::slotTokenDescriptionChanged);
    connect(checkScopeStatusRead,
            &QCheckBox::toggled,
            this,
            &DlgPrefRestServer::slotTokenScopesChanged);
    connect(checkScopeDecksRead,
            &QCheckBox::toggled,
            this,
            &DlgPrefRestServer::slotTokenScopesChanged);
    connect(checkScopeAutoDjRead,
            &QCheckBox::toggled,
            this,
            &DlgPrefRestServer::slotTokenScopesChanged);
    connect(checkScopeAutoDjWrite,
            &QCheckBox::toggled,
            this,
            &DlgPrefRestServer::slotTokenScopesChanged);
    connect(checkScopePlaylistsRead,
            &QCheckBox::toggled,
            this,
            &DlgPrefRestServer::slotTokenScopesChanged);
    connect(checkScopePlaylistsWrite,
            &QCheckBox::toggled,
            this,
            &DlgPrefRestServer::slotTokenScopesChanged);
    connect(checkScopeControlWrite,
            &QCheckBox::toggled,
            this,
            &DlgPrefRestServer::slotTokenScopesChanged);
    connect(comboBoxTokenExpiresPreset,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefRestServer::slotTokenExpirationPresetChanged);
    connect(dateTimeEditTokenExpires,
            &QDateTimeEdit::dateTimeChanged,
            this,
            &DlgPrefRestServer::slotTokenExpiresChanged);

    m_corsAllowlistToolTip = lineEditCorsAllowlist->toolTip();

    setScrollSafeGuardForAllInputWidgets(this);

    slotUpdate();
}

DlgPrefRestServer::~DlgPrefRestServer() = default;

void DlgPrefRestServer::slotApply() {
    const RestServerSettings::Values values = gatherValues();
    m_settings->set(values);
}

void DlgPrefRestServer::slotUpdate() {
    const RestServerSettings::Values values = m_settings->get();
    const RestServerSettings::Status status = m_settings->getStatus();
    loadValues(values);
    updateStatusLabels(status);
}

void DlgPrefRestServer::slotResetToDefaults() {
    const RestServerSettings::Values defaults = m_settings->defaults();
    loadValues(defaults);
    updateStatusLabels(RestServerSettings::Status{});
}

void DlgPrefRestServer::slotUseHttpsChanged(bool /*checked*/) {
    updateTlsState();
    updateAuthWarning();
    updateNetworkWarning();
    updateUrlLabels();
}

void DlgPrefRestServer::slotEnableHttpChanged(bool checked) {
    spinBoxHttpPort->setEnabled(checked);
    updateAuthWarning();
    updateNetworkWarning();
    updateUrlLabels();
}

void DlgPrefRestServer::slotAutoGenerateCertificateChanged(bool /*checked*/) {
    updateTlsState();
}

void DlgPrefRestServer::slotEnableRestServerChanged(bool checked) {
    groupBoxNetwork->setEnabled(checked);
    groupBoxAuthentication->setEnabled(checked);
    groupBoxTls->setEnabled(checked);
    updateAuthWarning();
    updateUnauthWarning();
    updateNetworkWarning();
    updatePresetWarning();
    updateUrlLabels();
}

void DlgPrefRestServer::slotPresetChanged(int index) {
    const QSignalBlocker hostBlocker(lineEditHost);

    if (index == kPresetLocal) {
        lineEditHost->setText(QStringLiteral("127.0.0.1"));
        checkBoxEnableHttp->setChecked(true);
        checkBoxUseHttps->setChecked(false);
        checkBoxAutoGenerateCertificate->setChecked(false);
        checkBoxRequireTls->setChecked(false);
        lineEditCorsAllowlist->setText(makeLoopbackCorsAllowlist(true, false));
    } else if (index == kPresetLocalHttps) {
        lineEditHost->setText(QStringLiteral("127.0.0.1"));
        checkBoxEnableHttp->setChecked(true);
        checkBoxUseHttps->setChecked(true);
        checkBoxAutoGenerateCertificate->setChecked(true);
        checkBoxRequireTls->setChecked(true);
        lineEditCorsAllowlist->setText(makeLoopbackCorsAllowlist(true, true));
    } else if (index == kPresetNetwork) {
        lineEditHost->setText(QStringLiteral("0.0.0.0"));
        checkBoxEnableHttp->setChecked(true);
        checkBoxUseHttps->setChecked(false);
        checkBoxAutoGenerateCertificate->setChecked(false);
        checkBoxRequireTls->setChecked(false);
        lineEditCorsAllowlist->clear();
    }

    updateTlsState();
    updateAuthWarning();
    updateNetworkWarning();
    updatePresetWarning();
    updateUrlLabels();
}

void DlgPrefRestServer::slotCorsAllowlistChanged(const QString& text) {
    const QString trimmedText = text.trimmed();
    QStringList invalidEntries;

    if (!trimmedText.isEmpty()) {
        const QStringList entries = text.split(',', Qt::KeepEmptyParts);
        for (const QString& entry : entries) {
            const QString trimmedEntry = entry.trimmed();
            if (trimmedEntry.isEmpty()) {
                invalidEntries << tr("<empty>");
                continue;
            }

            const QUrl url(trimmedEntry);
            const QString path = url.path();
            const int port = url.port();
            const bool invalid = !url.isValid() ||
                    url.scheme().isEmpty() ||
                    url.host().isEmpty() ||
                    !url.userInfo().isEmpty() ||
                    !url.query().isEmpty() ||
                    !url.fragment().isEmpty() ||
                    (!path.isEmpty() && path != QStringLiteral("/")) ||
                    (port != -1 && (port <= 0 || port > 65535));
            if (invalid) {
                invalidEntries << trimmedEntry;
            }
        }
    }

    if (invalidEntries.isEmpty()) {
        labelCorsAllowlistWarning->setVisible(false);
        lineEditCorsAllowlist->setToolTip(m_corsAllowlistToolTip);
    } else {
        const QString invalidList = invalidEntries.join(QStringLiteral(", "));
        labelCorsAllowlistWarning->setText(
                tr("Invalid origins: %1").arg(invalidList));
        labelCorsAllowlistWarning->setVisible(true);
        lineEditCorsAllowlist->setToolTip(
                tr("Invalid origins: %1").arg(invalidList));
    }
}

void DlgPrefRestServer::slotBrowseCertificate() {
    const QString fileName = browseForFile(tr("Select certificate"), lineEditCertPath->text());
    if (!fileName.isEmpty()) {
        lineEditCertPath->setText(fileName);
    }
}

void DlgPrefRestServer::slotBrowseKey() {
    const QString fileName = browseForFile(tr("Select private key"), lineEditKeyPath->text());
    if (!fileName.isEmpty()) {
        lineEditKeyPath->setText(fileName);
    }
}

void DlgPrefRestServer::loadValues(const RestServerSettings::Values& values) {
    checkBoxEnableRestServer->setChecked(values.enabled);
    checkBoxEnableHttp->setChecked(values.enableHttp);
    groupBoxNetwork->setEnabled(values.enabled);
    groupBoxAuthentication->setEnabled(values.enabled);
    groupBoxTls->setEnabled(values.enabled);

    lineEditHost->setText(values.host);
    spinBoxHttpPort->setValue(values.httpPort);
    spinBoxHttpsPort->setValue(values.httpsPort);
    lineEditCorsAllowlist->setText(values.corsAllowlist);
    spinBoxMaxRequestSize->setValue(qMax(1, (values.maxRequestBytes + 1023) / 1024));
    m_tokens = values.tokens;
    refreshTokenTable();
    updateSelection(m_tokens.isEmpty() ? -1 : 0);
    checkBoxUseHttps->setChecked(values.useHttps);
    checkBoxAutoGenerateCertificate->setChecked(values.autoGenerateCert);
    lineEditCertPath->setText(values.certificatePath);
    lineEditKeyPath->setText(values.privateKeyPath);
    checkBoxRequireTls->setChecked(values.requireTls);
    checkBoxAllowUnauthenticated->setChecked(values.allowUnauthenticated);

    slotEnableHttpChanged(values.enableHttp);
    updateTlsState();
    updateAuthWarning();
    updateUnauthWarning();
    updateNetworkWarning();
    updatePresetWarning();
    updateUrlLabels();
}

RestServerSettings::Values DlgPrefRestServer::gatherValues() const {
    RestServerSettings::Values values;
    values.enabled = checkBoxEnableRestServer->isChecked();
    values.enableHttp = checkBoxEnableHttp->isChecked();
    values.host = lineEditHost->text();
    values.httpPort = spinBoxHttpPort->value();
    values.httpsPort = spinBoxHttpsPort->value();
    values.corsAllowlist = lineEditCorsAllowlist->text();
    values.maxRequestBytes = spinBoxMaxRequestSize->value() * 1024;
    values.tokens = m_tokens;
    values.useHttps = checkBoxUseHttps->isChecked();
    values.autoGenerateCert = values.useHttps && checkBoxAutoGenerateCertificate->isChecked();
    values.certificatePath = lineEditCertPath->text();
    values.privateKeyPath = lineEditKeyPath->text();
    values.requireTls = checkBoxRequireTls->isChecked();
    values.allowUnauthenticated = checkBoxAllowUnauthenticated->isChecked();
    return values;
}

void DlgPrefRestServer::updateTlsState() {
    const bool useHttps = checkBoxUseHttps->isChecked();
    const bool autoGenerate = checkBoxAutoGenerateCertificate->isChecked();

    spinBoxHttpsPort->setEnabled(useHttps);

    checkBoxAutoGenerateCertificate->setEnabled(useHttps);
    checkBoxRequireTls->setEnabled(useHttps);
    if (useHttps) {
        checkBoxRequireTls->setToolTip(QString());
    } else {
        checkBoxRequireTls->setToolTip(tr("Enable HTTPS to require TLS for control routes."));
    }
    lineEditCertPath->setEnabled(useHttps && !autoGenerate);
    pushButtonBrowseCert->setEnabled(useHttps && !autoGenerate);
    lineEditKeyPath->setEnabled(useHttps && !autoGenerate);
    pushButtonBrowseKey->setEnabled(useHttps && !autoGenerate);
    labelTlsStatus->setEnabled(useHttps);
    labelTlsStatusIcon->setEnabled(useHttps);
    labelTlsCertificateStatus->setEnabled(useHttps);
    labelTlsCertificateIcon->setEnabled(useHttps);
    updateTlsCertificateStatus();
}

void DlgPrefRestServer::updateAuthWarning() {
    const bool hasTokens = !m_tokens.isEmpty();
    const bool showWarning = checkBoxEnableRestServer->isChecked() &&
            checkBoxEnableHttp->isChecked() &&
            hasTokens &&
            !checkBoxUseHttps->isChecked();
    labelAuthWarningIcon->setVisible(showWarning);
    labelAuthWarning->setVisible(showWarning);
}

void DlgPrefRestServer::updateUnauthWarning() {
    const bool showWarning = checkBoxEnableRestServer->isChecked() &&
            checkBoxAllowUnauthenticated->isChecked() &&
            m_tokens.isEmpty();
    labelUnauthWarningIcon->setVisible(showWarning);
    labelUnauthWarning->setVisible(showWarning);
}

void DlgPrefRestServer::updateNetworkWarning() {
    const bool showWarning = checkBoxEnableRestServer->isChecked() &&
            !checkBoxEnableHttp->isChecked() &&
            !checkBoxUseHttps->isChecked();
    labelNetworkWarningIcon->setVisible(showWarning);
    labelNetworkWarning->setVisible(showWarning);
}

void DlgPrefRestServer::updatePresetWarning() {
    labelPresetWarning->setVisible(comboBoxPreset->currentIndex() == kPresetNetwork);
}

void DlgPrefRestServer::updateUrlLabels() {
    const bool restEnabled = checkBoxEnableRestServer->isChecked();
    const bool httpEnabled = restEnabled && checkBoxEnableHttp->isChecked();
    const bool httpsEnabled = restEnabled && checkBoxUseHttps->isChecked();

    if (httpEnabled) {
        labelHttpUrlValue->setText(makeHttpUrl());
        pushButtonCopyHttpUrl->setEnabled(true);
        pushButtonCopyHttpUrl->setToolTip(QString());
    } else {
        labelHttpUrlValue->setText(tr("Disabled"));
        pushButtonCopyHttpUrl->setEnabled(false);
        pushButtonCopyHttpUrl->setToolTip(tr("HTTP listener is disabled, so there is no URL to copy."));
    }

    if (httpsEnabled) {
        labelHttpsUrlValue->setText(makeHttpsUrl());
        pushButtonCopyHttpsUrl->setEnabled(true);
        pushButtonCopyHttpsUrl->setToolTip(QString());
    } else {
        labelHttpsUrlValue->setText(tr("Disabled"));
        pushButtonCopyHttpsUrl->setEnabled(false);
        pushButtonCopyHttpsUrl->setToolTip(tr("HTTPS listener is disabled, so there is no URL to copy."));
    }
}

QString DlgPrefRestServer::makeHttpUrl() const {
    const QString host = lineEditHost->text().trimmed();
    const QString hostDisplay = host.isEmpty() ? QStringLiteral("localhost") : host;
    return tr("http://%1:%2").arg(hostDisplay).arg(spinBoxHttpPort->value());
}

QString DlgPrefRestServer::makeHttpsUrl() const {
    const QString host = lineEditHost->text().trimmed();
    const QString hostDisplay = host.isEmpty() ? QStringLiteral("localhost") : host;
    return tr("https://%1:%2").arg(hostDisplay).arg(spinBoxHttpsPort->value());
}

QString DlgPrefRestServer::makeLoopbackCorsAllowlist(bool includeHttp, bool includeHttps) const {
    QStringList allowlist;
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    allowlist.reserve(6);
#endif
    if (includeHttp) {
        const int port = spinBoxHttpPort->value();
        allowlist << QStringLiteral("http://localhost:%1").arg(port)
                  << QStringLiteral("http://127.0.0.1:%1").arg(port)
                  << QStringLiteral("http://[::1]:%1").arg(port);
    }
    if (includeHttps) {
        const int port = spinBoxHttpsPort->value();
        allowlist << QStringLiteral("https://localhost:%1").arg(port)
                  << QStringLiteral("https://127.0.0.1:%1").arg(port)
                  << QStringLiteral("https://[::1]:%1").arg(port);
    }
    return allowlist.join(QStringLiteral(", "));
}

void DlgPrefRestServer::updateStatusLabels(const RestServerSettings::Status& status) {
    const bool showStatus = !status.lastError.isEmpty();
    labelStatusIcon->setVisible(showStatus);
    labelStatus->setVisible(showStatus);
    labelStatus->setText(status.lastError);

    const bool showTlsStatus = !status.tlsError.isEmpty() || status.certificateGenerated;
    labelTlsStatusIcon->setVisible(showTlsStatus);
    labelTlsStatus->setVisible(showTlsStatus);
    if (status.certificateGenerated && status.tlsError.isEmpty()) {
        labelTlsStatus->setText(tr("A self-signed certificate was generated automatically."));
    } else {
        labelTlsStatus->setText(status.tlsError);
    }
    updateTlsCertificateStatus();

    updateNetworkWarning();
    updatePresetWarning();
}

void DlgPrefRestServer::updateTlsCertificateStatus() {
    const bool useHttps = checkBoxUseHttps->isChecked();
    if (!useHttps) {
        labelTlsCertificateStatus->setVisible(false);
        labelTlsCertificateIcon->setVisible(false);
        return;
    }

#if QT_CONFIG(ssl)
    QString certificatePath = lineEditCertPath->text().trimmed();
    if (certificatePath.isEmpty() && m_settings) {
        certificatePath = m_settings->defaultCertificatePath();
    }
    if (certificatePath.isEmpty() || !QFileInfo::exists(certificatePath)) {
        labelTlsCertificateStatus->setVisible(false);
        labelTlsCertificateIcon->setVisible(false);
        return;
    }

    const QList<QSslCertificate> certificates =
            QSslCertificate::fromPath(certificatePath, QSsl::Pem);
    if (certificates.isEmpty()) {
        labelTlsCertificateStatus->setVisible(false);
        labelTlsCertificateIcon->setVisible(false);
        return;
    }

    const QSslCertificate certificate = certificates.first();
    if (certificate.isNull()) {
        labelTlsCertificateStatus->setVisible(false);
        labelTlsCertificateIcon->setVisible(false);
        return;
    }

    QString issuer = certificate.issuerInfo(QSslCertificate::CommonName).join(QStringLiteral(", "));
    if (issuer.isEmpty()) {
        issuer = certificate.issuerInfo(QSslCertificate::Organization).join(QStringLiteral(", "));
    }
    if (issuer.isEmpty()) {
        issuer = tr("Unknown issuer");
    }

    const QDateTime expiryUtc = certificate.expiryDate();
    const QString expiryText = expiryUtc.isValid()
            ? QLocale().toString(expiryUtc.toLocalTime(), QLocale::ShortFormat)
            : tr("Unknown");

    labelTlsCertificateStatus->setText(
            tr("Issuer: %1\nExpires: %2").arg(issuer, expiryText));
    labelTlsCertificateStatus->setVisible(true);

    bool showWarning = false;
    if (expiryUtc.isValid()) {
        const QDateTime now = QDateTime::currentDateTimeUtc();
        showWarning = expiryUtc <= now || expiryUtc <= now.addDays(kCertificateExpiryWarningDays);
    }
    labelTlsCertificateIcon->setVisible(showWarning);
#else
    labelTlsCertificateStatus->setVisible(false);
    labelTlsCertificateIcon->setVisible(false);
#endif
}

QString DlgPrefRestServer::browseForFile(const QString& title, const QString& startDirectory) const {
    return QFileDialog::getOpenFileName(const_cast<QWidget*>(static_cast<const QWidget*>(this)),
            title,
            startDirectory);
}

void DlgPrefRestServer::slotAddToken() {
    if (m_tokens.size() >= RestServerSettings::kMaxTokens) {
        return;
    }

    RestServerToken token;
    token.value = makeToken();
    token.scopes = mixxx::network::rest::scopes::allScopes();
    token.createdUtc = QDateTime::currentDateTimeUtc();
    m_tokens.append(token);
    refreshTokenTable();
    updateSelection(m_tokens.size() - 1);
    updateAuthWarning();
    updateUnauthWarning();
}

void DlgPrefRestServer::slotRemoveToken() {
    if (m_selectedToken < 0 || m_selectedToken >= m_tokens.size()) {
        return;
    }
    m_tokens.removeAt(m_selectedToken);
    refreshTokenTable();
    updateSelection(qMin(m_selectedToken, m_tokens.size() - 1));
    updateAuthWarning();
    updateUnauthWarning();
}

void DlgPrefRestServer::slotRegenerateToken() {
    RestServerToken* token = selectedToken();
    if (!token) {
        return;
    }
    token->value = makeToken();
    token->createdUtc = QDateTime::currentDateTimeUtc();
    refreshTokenTable();
    syncEditorsFromSelection();
    updateAuthWarning();
}

void DlgPrefRestServer::slotTokenSelectionChanged() {
    const QModelIndexList rows = tableTokens->selectionModel()->selectedRows();
    if (rows.isEmpty()) {
        updateSelection(-1);
        return;
    }
    updateSelection(rows.first().row());
}

void DlgPrefRestServer::slotTokenDescriptionChanged(const QString& text) {
    RestServerToken* token = selectedToken();
    if (!token) {
        return;
    }
    token->description = text;
    refreshTokenTable();
}

void DlgPrefRestServer::slotTokenScopesChanged() {
    RestServerToken* token = selectedToken();
    if (!token) {
        return;
    }
    token->scopes = selectedScopes();
    refreshTokenTable();
}

void DlgPrefRestServer::slotTokenExpirationPresetChanged(int index) {
    RestServerToken* token = selectedToken();
    if (!token) {
        return;
    }

    QDateTime selectedDateTime;
    bool isCustom = false;
    switch (index) {
    case kExpirationPresetNever:
        selectedDateTime = dateTimeEditTokenExpires->minimumDateTime();
        break;
    case kExpirationPresetOneMonth:
        selectedDateTime = QDateTime::currentDateTimeUtc().addMonths(1);
        break;
    case kExpirationPresetThreeMonths:
        selectedDateTime = QDateTime::currentDateTimeUtc().addMonths(3);
        break;
    case kExpirationPresetOneYear:
        selectedDateTime = QDateTime::currentDateTimeUtc().addYears(1);
        break;
    case kExpirationPresetCustom:
        isCustom = true;
        selectedDateTime = QDateTime(
                QDate::currentDate(),
                QTime(0, 0),
                QTimeZone::utc());
        break;
    default:
        selectedDateTime = dateTimeEditTokenExpires->minimumDateTime();
        break;
    }

    dateTimeEditTokenExpires->setEnabled(isCustom);
    dateTimeEditTokenExpires->setDateTime(selectedDateTime);
}

void DlgPrefRestServer::slotTokenExpiresChanged(const QDateTime& dateTime) {
    RestServerToken* token = selectedToken();
    if (!token) {
        return;
    }
    if (dateTime == dateTimeEditTokenExpires->minimumDateTime()) {
        token->expiresUtc.reset();
    } else {
        token->expiresUtc = dateTime.toUTC();
    }
    labelTokenExpiresValue->setText(token->expiresUtc.has_value()
                    ? token->expiresUtc->toString(Qt::ISODate)
                    : tr("Never"));
    refreshTokenTable();
}

void DlgPrefRestServer::slotCopyToken() {
    if (m_fullToken.isEmpty()) {
        return;
    }
    QGuiApplication::clipboard()->setText(m_fullToken);
}

void DlgPrefRestServer::slotToggleTokenVisibility() {
    if (m_fullToken.isEmpty()) {
        return;
    }
    m_isTokenVisible = !m_isTokenVisible;
    updateTokenVisibility();
}

QString DlgPrefRestServer::makeToken() const {
    return QUuid::createUuid().toString(QUuid::WithoutBraces).remove('-');
}

void DlgPrefRestServer::refreshTokenTable() {
    tableTokens->setRowCount(m_tokens.size());
    int row = 0;
    for (const auto& token : m_tokens) {
        const QString shortToken = token.value.left(8);
        const QString expires = token.expiresUtc.has_value()
                ? token.expiresUtc->toString(Qt::ISODate)
                : tr("Never");
        const QString created = token.createdUtc.toString(Qt::ISODate);
        const QString lastUsed = token.lastUsedUtc.has_value()
                ? token.lastUsedUtc->toString(Qt::ISODate)
                : tr("Never");
        const QString scopes = token.scopes.isEmpty()
                ? tr("None")
                : token.scopes.join(QStringLiteral(", "));

        tableTokens->setItem(row, 0, new QTableWidgetItem(shortToken));
        tableTokens->setItem(row, 1, new QTableWidgetItem(token.description));
        tableTokens->setItem(row, 2, new QTableWidgetItem(scopes));
        tableTokens->setItem(row, 3, new QTableWidgetItem(expires));
        tableTokens->setItem(row, 4, new QTableWidgetItem(created));
        tableTokens->setItem(row, 5, new QTableWidgetItem(lastUsed));
        ++row;
    }
    tableTokens->resizeColumnsToContents();
    if (m_selectedToken >= 0 && m_selectedToken < m_tokens.size()) {
        tableTokens->selectRow(m_selectedToken);
    }
}

void DlgPrefRestServer::updateSelection(int row) {
    m_selectedToken = row;
    if (row >= 0 && row < m_tokens.size()) {
        tableTokens->selectRow(row);
    } else {
        tableTokens->clearSelection();
    }
    syncEditorsFromSelection();
}

void DlgPrefRestServer::syncEditorsFromSelection() {
    const RestServerToken* token = selectedToken();
    const bool hasToken = token != nullptr;
    lineEditTokenValue->setEnabled(hasToken);
    lineEditTokenDescription->setEnabled(hasToken);
    widgetTokenScopes->setEnabled(hasToken);
    dateTimeEditTokenExpires->setEnabled(hasToken);
    comboBoxTokenExpiresPreset->setEnabled(hasToken);
    pushButtonRemoveToken->setEnabled(hasToken);
    pushButtonRegenerateToken->setEnabled(hasToken);
    pushButtonCopyToken->setEnabled(hasToken);
    pushButtonToggleToken->setEnabled(hasToken);

    if (!hasToken) {
        m_fullToken.clear();
        m_isTokenVisible = false;
        lineEditTokenValue->clear();
        lineEditTokenDescription->clear();
        updateScopeEditors({});
        labelTokenExpiresValue->setText(tr("Never"));
        {
            const QSignalBlocker presetBlocker(comboBoxTokenExpiresPreset);
            comboBoxTokenExpiresPreset->setCurrentIndex(kExpirationPresetNever);
        }
        {
            const QSignalBlocker expiresBlocker(dateTimeEditTokenExpires);
            dateTimeEditTokenExpires->setDateTime(dateTimeEditTokenExpires->minimumDateTime());
        }
        dateTimeEditTokenExpires->setEnabled(false);
        updateTokenVisibility();
        return;
    }

    m_fullToken = token->value;
    m_isTokenVisible = false;
    lineEditTokenValue->setText(m_fullToken);
    lineEditTokenDescription->setText(token->description);
    updateScopeEditors(token->scopes);
    labelTokenExpiresValue->setText(token->expiresUtc.has_value()
                    ? token->expiresUtc->toString(Qt::ISODate)
                    : tr("Never"));
    {
        const QSignalBlocker presetBlocker(comboBoxTokenExpiresPreset);
        comboBoxTokenExpiresPreset->setCurrentIndex(kExpirationPresetNever);
    }
    {
        const QSignalBlocker expiresBlocker(dateTimeEditTokenExpires);
        dateTimeEditTokenExpires->setDateTime(dateTimeEditTokenExpires->minimumDateTime());
    }
    dateTimeEditTokenExpires->setEnabled(false);
    updateTokenVisibility();
}

void DlgPrefRestServer::updateTokenVisibility() {
    lineEditTokenValue->setEchoMode(m_isTokenVisible ? QLineEdit::Normal : QLineEdit::Password);
    pushButtonToggleToken->setText(m_isTokenVisible ? tr("Hide") : tr("Show"));
}

QStringList DlgPrefRestServer::selectedScopes() const {
    QStringList scopes;
    if (checkScopeStatusRead->isChecked()) {
        scopes.append(mixxx::network::rest::scopes::kStatusRead);
    }
    if (checkScopeDecksRead->isChecked()) {
        scopes.append(mixxx::network::rest::scopes::kDecksRead);
    }
    if (checkScopeAutoDjRead->isChecked()) {
        scopes.append(mixxx::network::rest::scopes::kAutoDjRead);
    }
    if (checkScopeAutoDjWrite->isChecked()) {
        scopes.append(mixxx::network::rest::scopes::kAutoDjWrite);
    }
    if (checkScopePlaylistsRead->isChecked()) {
        scopes.append(mixxx::network::rest::scopes::kPlaylistsRead);
    }
    if (checkScopePlaylistsWrite->isChecked()) {
        scopes.append(mixxx::network::rest::scopes::kPlaylistsWrite);
    }
    if (checkScopeControlWrite->isChecked()) {
        scopes.append(mixxx::network::rest::scopes::kControlWrite);
    }
    return scopes;
}

void DlgPrefRestServer::updateScopeEditors(const QStringList& scopes) {
    const QSignalBlocker statusBlocker(checkScopeStatusRead);
    const QSignalBlocker decksBlocker(checkScopeDecksRead);
    const QSignalBlocker autoDjReadBlocker(checkScopeAutoDjRead);
    const QSignalBlocker autoDjWriteBlocker(checkScopeAutoDjWrite);
    const QSignalBlocker playlistsReadBlocker(checkScopePlaylistsRead);
    const QSignalBlocker playlistsWriteBlocker(checkScopePlaylistsWrite);
    const QSignalBlocker controlWriteBlocker(checkScopeControlWrite);

    checkScopeStatusRead->setChecked(
            scopes.contains(mixxx::network::rest::scopes::kStatusRead, Qt::CaseInsensitive));
    checkScopeDecksRead->setChecked(
            scopes.contains(mixxx::network::rest::scopes::kDecksRead, Qt::CaseInsensitive));
    checkScopeAutoDjRead->setChecked(
            scopes.contains(mixxx::network::rest::scopes::kAutoDjRead, Qt::CaseInsensitive));
    checkScopeAutoDjWrite->setChecked(
            scopes.contains(mixxx::network::rest::scopes::kAutoDjWrite, Qt::CaseInsensitive));
    checkScopePlaylistsRead->setChecked(
            scopes.contains(mixxx::network::rest::scopes::kPlaylistsRead, Qt::CaseInsensitive));
    checkScopePlaylistsWrite->setChecked(
            scopes.contains(mixxx::network::rest::scopes::kPlaylistsWrite, Qt::CaseInsensitive));
    checkScopeControlWrite->setChecked(
            scopes.contains(mixxx::network::rest::scopes::kControlWrite, Qt::CaseInsensitive));
}

RestServerToken* DlgPrefRestServer::selectedToken() {
    if (m_selectedToken < 0 || m_selectedToken >= m_tokens.size()) {
        return nullptr;
    }
    return &m_tokens[m_selectedToken];
}
