#pragma once

#include <memory>

#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefwebserverdlg.h"
#include "preferences/webserversettings.h"

class QWidget;

class DlgPrefWebServer : public DlgPreferencePage, public Ui::DlgPrefWebServerDlg {
    Q_OBJECT
  public:
    DlgPrefWebServer(QWidget* parent, std::shared_ptr<WebServerSettings> pSettings);
    ~DlgPrefWebServer() override;

  signals:
    void webServerSettingsChanged(const WebServerSettings::Values& settings);
    void webServerCertificateGenerationRequested(const WebServerSettings::Values& settings);

  public slots:
    void slotApply() override;
    void slotUpdate() override;
    void slotResetToDefaults() override;

  private slots:
    void slotBrowseCertificate();
    void slotBrowseKey();
    void slotBrowseCaBundle();
    void slotEnableAuthenticationChanged(bool checked);
    void slotUseHttpsChanged(bool checked);
    void slotAutoGenerateCertificateChanged(bool checked);
    void slotEnableWebServerChanged(bool checked);

  private:
    void loadValues(const WebServerSettings::Values& values);
    WebServerSettings::Values gatherValues() const;
    void updateTlsState();
    void updateAuthenticationState();
    void updateAuthWarning();
    QString browseForFile(const QString& title, const QString& startDirectory) const;

    std::shared_ptr<WebServerSettings> m_pSettings;
};
