#pragma once

#ifdef MIXXX_HAS_HTTP_SERVER

#include <memory>

#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefrestserverdlg.h"
#include "preferences/restserversettings.h"

class QWidget;

class DlgPrefRestServer : public DlgPreferencePage, public Ui::DlgPrefRestServerDlg {
    Q_OBJECT
  public:
    DlgPrefRestServer(QWidget* parent, std::shared_ptr<RestServerSettings> settings);
    ~DlgPrefRestServer() override;

  public slots:
    void slotApply() override;
    void slotUpdate() override;
    void slotResetToDefaults() override;

  private slots:
    void slotUseHttpsChanged(bool checked);
    void slotAutoGenerateCertificateChanged(bool checked);
    void slotEnableRestServerChanged(bool checked);
    void slotEnableHttpChanged(bool checked);
    void slotBrowseCertificate();
    void slotBrowseKey();
    void slotTokenChanged(const QString& token);

  private:
    void loadValues(const RestServerSettings::Values& values);
    RestServerSettings::Values gatherValues() const;
    void updateTlsState();
    void updateAuthWarning();
    void updateStatusLabels(const RestServerSettings::Status& status);
    QString browseForFile(const QString& title, const QString& startDirectory) const;

    std::shared_ptr<RestServerSettings> m_settings;
};

#endif // MIXXX_HAS_HTTP_SERVER
