#include "preferences/dialog/dlgprefrestserver.h"

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QFileDialog>
#include <QIcon>
#include <limits>
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

    labelAuthWarningIcon->setPixmap(QIcon(kWarningIconPath).pixmap(20, 20));
    labelStatusIcon->setPixmap(QIcon(kWarningIconPath).pixmap(20, 20));
    labelTlsStatusIcon->setPixmap(QIcon(kWarningIconPath).pixmap(20, 20));

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
    connect(lineEditAuthToken,
            &QLineEdit::textChanged,
            this,
            &DlgPrefRestServer::slotTokenChanged);

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

void DlgPrefRestServer::slotTokenChanged(const QString& /*token*/) {
    updateAuthWarning();
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
    lineEditAuthToken->setText(values.authToken);
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
    values.authToken = lineEditAuthToken->text();
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
    const bool showWarning = checkBoxEnableRestServer->isChecked() &&
            checkBoxEnableHttp->isChecked() &&
            !lineEditAuthToken->text().isEmpty() &&
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

#endif // MIXXX_HAS_HTTP_SERVER
