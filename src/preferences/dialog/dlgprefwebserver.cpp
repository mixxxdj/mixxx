#include "preferences/dialog/dlgprefwebserver.h"

#include <QFileDialog>
#include <QIcon>
#include <QMetaType>
#include <QVariant>

#include "moc_dlgprefwebserver.cpp"

DlgPrefWebServer::DlgPrefWebServer(QWidget* parent, std::shared_ptr<WebServerSettings> pSettings)
        : DlgPreferencePage(parent),
          m_pSettings(std::move(pSettings)) {
    setupUi(this);
    createLinkColor();

    qRegisterMetaType<WebServerSettings::BindAddress>();
    qRegisterMetaType<WebServerSettings::Values>();

    comboBoxBindAddress->addItem(tr("Localhost (127.0.0.1)"),
            QVariant::fromValue(WebServerSettings::BindAddress::Localhost));
    comboBoxBindAddress->addItem(tr("All interfaces (0.0.0.0)"),
            QVariant::fromValue(WebServerSettings::BindAddress::Any));

    spinBoxPort->setMinimum(1);
    spinBoxPort->setMaximum(65535);

    labelAuthWarningIcon->setPixmap(QIcon(kWarningIconPath).pixmap(20, 20));

    connect(checkBoxEnableWebServer,
            &QCheckBox::toggled,
            this,
            &DlgPrefWebServer::slotEnableWebServerChanged);
    connect(checkBoxEnableAuthentication,
            &QCheckBox::toggled,
            this,
            &DlgPrefWebServer::slotEnableAuthenticationChanged);
    connect(checkBoxUseHttps,
            &QCheckBox::toggled,
            this,
            &DlgPrefWebServer::slotUseHttpsChanged);
    connect(checkBoxAutoGenerateCertificate,
            &QCheckBox::toggled,
            this,
            &DlgPrefWebServer::slotAutoGenerateCertificateChanged);
    connect(pushButtonBrowseCert,
            &QPushButton::clicked,
            this,
            &DlgPrefWebServer::slotBrowseCertificate);
    connect(pushButtonBrowseKey,
            &QPushButton::clicked,
            this,
            &DlgPrefWebServer::slotBrowseKey);
    connect(pushButtonBrowseCaBundle,
            &QPushButton::clicked,
            this,
            &DlgPrefWebServer::slotBrowseCaBundle);

    setScrollSafeGuardForAllInputWidgets(this);

    slotUpdate();
}

DlgPrefWebServer::~DlgPrefWebServer() {
}

void DlgPrefWebServer::slotApply() {
    const WebServerSettings::Values values = gatherValues();
    m_pSettings->set(values);
    emit webServerSettingsChanged(values);
    if (values.useHttps && values.autoGenerateCert) {
        emit webServerCertificateGenerationRequested(values);
    }
}

void DlgPrefWebServer::slotUpdate() {
    const WebServerSettings::Values values = m_pSettings->get();
    loadValues(values);
}

void DlgPrefWebServer::slotResetToDefaults() {
    const WebServerSettings::Values defaults = m_pSettings->defaults();
    loadValues(defaults);
}

void DlgPrefWebServer::slotBrowseCertificate() {
    const QString fileName = browseForFile(tr("Select certificate"),
            lineEditCertPath->text());
    if (!fileName.isEmpty()) {
        lineEditCertPath->setText(fileName);
    }
}

void DlgPrefWebServer::slotBrowseKey() {
    const QString fileName = browseForFile(tr("Select private key"),
            lineEditKeyPath->text());
    if (!fileName.isEmpty()) {
        lineEditKeyPath->setText(fileName);
    }
}

void DlgPrefWebServer::slotBrowseCaBundle() {
    const QString fileName = browseForFile(tr("Select CA bundle"),
            lineEditCaBundlePath->text());
    if (!fileName.isEmpty()) {
        lineEditCaBundlePath->setText(fileName);
    }
}

void DlgPrefWebServer::slotEnableAuthenticationChanged(bool /*checked*/) {
    updateAuthenticationState();
}

void DlgPrefWebServer::slotUseHttpsChanged(bool /*checked*/) {
    updateTlsState();
    updateAuthWarning();
}

void DlgPrefWebServer::slotAutoGenerateCertificateChanged(bool /*checked*/) {
    updateTlsState();
}

void DlgPrefWebServer::slotEnableWebServerChanged(bool checked) {
    groupBoxNetwork->setEnabled(checked);
    groupBoxAuthentication->setEnabled(checked);
    groupBoxTls->setEnabled(checked);
    updateAuthWarning();
}

void DlgPrefWebServer::loadValues(const WebServerSettings::Values& values) {
    checkBoxEnableWebServer->setChecked(values.enabled);
    groupBoxNetwork->setEnabled(values.enabled);
    groupBoxAuthentication->setEnabled(values.enabled);
    groupBoxTls->setEnabled(values.enabled);

    const QVariant bindAddressVariant = QVariant::fromValue(values.bindAddress);
    const int bindIndex = comboBoxBindAddress->findData(bindAddressVariant);
    if (bindIndex >= 0) {
        comboBoxBindAddress->setCurrentIndex(bindIndex);
    }
    spinBoxPort->setValue(values.port);

    checkBoxEnableAuthentication->setChecked(values.authenticationEnabled);
    lineEditUsername->setText(values.username);
    lineEditPassword->setText(values.password);
    checkBoxUseHttps->setChecked(values.useHttps);
    checkBoxAutoGenerateCertificate->setChecked(values.autoGenerateCert);
    lineEditCertPath->setText(values.certPath);
    lineEditKeyPath->setText(values.keyPath);
    lineEditCaBundlePath->setText(values.caBundlePath);

    updateAuthenticationState();
    updateTlsState();
    updateAuthWarning();
}

WebServerSettings::Values DlgPrefWebServer::gatherValues() const {
    WebServerSettings::Values values;
    values.enabled = checkBoxEnableWebServer->isChecked();
    values.bindAddress = comboBoxBindAddress->currentData().value<WebServerSettings::BindAddress>();
    values.port = spinBoxPort->value();
    values.authenticationEnabled = checkBoxEnableAuthentication->isChecked();
    values.username = lineEditUsername->text();
    values.password = lineEditPassword->text();
    values.useHttps = checkBoxUseHttps->isChecked();
    values.autoGenerateCert = values.useHttps &&
            checkBoxAutoGenerateCertificate->isChecked();
    values.certPath = lineEditCertPath->text();
    values.keyPath = lineEditKeyPath->text();
    values.caBundlePath = lineEditCaBundlePath->text();
    return values;
}

void DlgPrefWebServer::updateTlsState() {
    const bool useHttps = checkBoxUseHttps->isChecked();
    const bool autoGenerate = checkBoxAutoGenerateCertificate->isChecked();

    checkBoxAutoGenerateCertificate->setEnabled(useHttps);
    lineEditCertPath->setEnabled(useHttps && !autoGenerate);
    pushButtonBrowseCert->setEnabled(useHttps && !autoGenerate);
    lineEditKeyPath->setEnabled(useHttps && !autoGenerate);
    pushButtonBrowseKey->setEnabled(useHttps && !autoGenerate);
    lineEditCaBundlePath->setEnabled(useHttps);
    pushButtonBrowseCaBundle->setEnabled(useHttps);
}

void DlgPrefWebServer::updateAuthenticationState() {
    const bool authEnabled = checkBoxEnableAuthentication->isChecked();
    lineEditUsername->setEnabled(authEnabled);
    lineEditPassword->setEnabled(authEnabled);
    updateAuthWarning();
}

void DlgPrefWebServer::updateAuthWarning() {
    const bool showWarning = checkBoxEnableWebServer->isChecked() &&
            checkBoxEnableAuthentication->isChecked() &&
            !checkBoxUseHttps->isChecked();
    labelAuthWarningIcon->setVisible(showWarning);
    labelAuthWarning->setVisible(showWarning);
}

QString DlgPrefWebServer::browseForFile(const QString& title, const QString& startDirectory) const {
    return QFileDialog::getOpenFileName(this, title, startDirectory);
}
