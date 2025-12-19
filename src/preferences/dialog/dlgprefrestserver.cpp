#include "preferences/dialog/dlgprefrestserver.h"

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QAbstractItemView>
#include <QDateTime>
#include <QFileDialog>
#include <QHeaderView>
#include <QIcon>
#include <QTableWidgetItem>
#include <limits>
#include <QUuid>
#include <QtGlobal>

#include "moc_dlgprefrestserver.cpp"

DlgPrefRestServer::DlgPrefRestServer(QWidget* parent, std::shared_ptr<RestServerSettings> settings)
        : DlgPreferencePage(parent),
          m_settings(std::move(settings)) {
    setupUi(this);
    createLinkColor();

    spinBoxHttpPort->setMinimum(1);
    spinBoxHttpPort->setMaximum(std::numeric_limits<quint16>::max());
    spinBoxHttpsPort->setMinimum(1);
    spinBoxHttpsPort->setMaximum(std::numeric_limits<quint16>::max());
    spinBoxMaxRequestSize->setMinimum(1);
    spinBoxMaxRequestSize->setMaximum(std::numeric_limits<int>::max() / 1024);

    labelAuthWarningIcon->setPixmap(QIcon(kWarningIconPath).pixmap(20, 20));
    labelStatusIcon->setPixmap(QIcon(kWarningIconPath).pixmap(20, 20));
    labelTlsStatusIcon->setPixmap(QIcon(kWarningIconPath).pixmap(20, 20));

    tableTokens->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableTokens->setSelectionMode(QAbstractItemView::SingleSelection);
    tableTokens->horizontalHeader()->setStretchLastSection(true);
    dateTimeEditTokenExpires->setMinimumDateTime(QDateTime(QDate(1970, 1, 1), QTime(0, 0), Qt::UTC));
    dateTimeEditTokenExpires->setTimeSpec(Qt::UTC);

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
    connect(tableTokens->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &DlgPrefRestServer::slotTokenSelectionChanged);
    connect(lineEditTokenDescription,
            &QLineEdit::textChanged,
            this,
            &DlgPrefRestServer::slotTokenDescriptionChanged);
    connect(comboTokenPermission,
            qOverload<int>(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefRestServer::slotTokenPermissionChanged);
    connect(dateTimeEditTokenExpires,
            &QDateTimeEdit::dateTimeChanged,
            this,
            &DlgPrefRestServer::slotTokenExpiresChanged);

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
}

void DlgPrefRestServer::slotEnableHttpChanged(bool checked) {
    spinBoxHttpPort->setEnabled(checked);
    updateAuthWarning();
}

void DlgPrefRestServer::slotAutoGenerateCertificateChanged(bool /*checked*/) {
    updateTlsState();
}

void DlgPrefRestServer::slotEnableRestServerChanged(bool checked) {
    groupBoxNetwork->setEnabled(checked);
    groupBoxAuthentication->setEnabled(checked);
    groupBoxTls->setEnabled(checked);
    updateAuthWarning();
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

    slotEnableHttpChanged(values.enableHttp);
    updateTlsState();
    updateAuthWarning();
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
    return values;
}

void DlgPrefRestServer::updateTlsState() {
    const bool useHttps = checkBoxUseHttps->isChecked();
    const bool autoGenerate = checkBoxAutoGenerateCertificate->isChecked();

    spinBoxHttpsPort->setEnabled(useHttps);

    checkBoxAutoGenerateCertificate->setEnabled(useHttps);
    checkBoxRequireTls->setEnabled(useHttps || checkBoxRequireTls->isChecked());
    lineEditCertPath->setEnabled(useHttps && !autoGenerate);
    pushButtonBrowseCert->setEnabled(useHttps && !autoGenerate);
    lineEditKeyPath->setEnabled(useHttps && !autoGenerate);
    pushButtonBrowseKey->setEnabled(useHttps && !autoGenerate);
    labelTlsStatus->setEnabled(useHttps);
    labelTlsStatusIcon->setEnabled(useHttps);
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
}

QString DlgPrefRestServer::browseForFile(const QString& title, const QString& startDirectory) const {
    return QFileDialog::getOpenFileName(this, title, startDirectory);
}

void DlgPrefRestServer::slotAddToken() {
    if (m_tokens.size() >= RestServerSettings::kMaxTokens) {
        return;
    }

    RestServerToken token;
    token.value = makeToken();
    token.permission = QStringLiteral("read");
    token.createdUtc = QDateTime::currentDateTimeUtc();
    m_tokens.append(token);
    refreshTokenTable();
    updateSelection(m_tokens.size() - 1);
    updateAuthWarning();
}

void DlgPrefRestServer::slotRemoveToken() {
    if (m_selectedToken < 0 || m_selectedToken >= m_tokens.size()) {
        return;
    }
    m_tokens.removeAt(m_selectedToken);
    refreshTokenTable();
    updateSelection(qMin(m_selectedToken, m_tokens.size() - 1));
    updateAuthWarning();
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

void DlgPrefRestServer::slotTokenPermissionChanged(int index) {
    RestServerToken* token = selectedToken();
    if (!token) {
        return;
    }
    token->permission = comboTokenPermission->itemText(index);
    refreshTokenTable();
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
    refreshTokenTable();
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

        tableTokens->setItem(row, 0, new QTableWidgetItem(shortToken));
        tableTokens->setItem(row, 1, new QTableWidgetItem(token.description));
        tableTokens->setItem(row, 2, new QTableWidgetItem(token.permission));
        tableTokens->setItem(row, 3, new QTableWidgetItem(expires));
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
    comboTokenPermission->setEnabled(hasToken);
    dateTimeEditTokenExpires->setEnabled(hasToken);
    pushButtonRemoveToken->setEnabled(hasToken);
    pushButtonRegenerateToken->setEnabled(hasToken);

    if (!hasToken) {
        lineEditTokenValue->clear();
        lineEditTokenDescription->clear();
        comboTokenPermission->setCurrentIndex(0);
        dateTimeEditTokenExpires->setDateTime(dateTimeEditTokenExpires->minimumDateTime());
        return;
    }

    lineEditTokenValue->setText(token->value);
    lineEditTokenDescription->setText(token->description);
    const int permissionIndex = comboTokenPermission->findText(token->permission);
    comboTokenPermission->setCurrentIndex(permissionIndex < 0 ? 0 : permissionIndex);
    if (token->expiresUtc.has_value()) {
        dateTimeEditTokenExpires->setDateTime(token->expiresUtc.value());
    } else {
        dateTimeEditTokenExpires->setDateTime(dateTimeEditTokenExpires->minimumDateTime());
    }
}

RestServerToken* DlgPrefRestServer::selectedToken() {
    if (m_selectedToken < 0 || m_selectedToken >= m_tokens.size()) {
        return nullptr;
    }
    return &m_tokens[m_selectedToken];
}

#endif // MIXXX_HAS_HTTP_SERVER
