#pragma once

#include <memory>
#include <QDateTime>
#include <QList>

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
    void slotPresetChanged(int index);
    void slotCorsAllowlistChanged(const QString& text);
    void slotBrowseCertificate();
    void slotBrowseKey();
    void slotShowCertificateDetails();
    void slotRegenerateCertificate();
    void slotAddToken();
    void slotRemoveToken();
    void slotRegenerateToken();
    void slotTokenSelectionChanged();
    void slotTokenDescriptionChanged(const QString& text);
    void slotTokenScopesChanged();
    void slotTokenExpirationPresetChanged(int index);
    void slotTokenExpiresChanged(const QDateTime& dateTime);
    void slotCopyToken();
    void slotToggleTokenVisibility();

  private:
    void loadValues(const RestServerSettings::Values& values);
    RestServerSettings::Values gatherValues() const;
    void updateTlsState();
    void updateAuthWarning();
    void updateAllowUnauthenticatedState();
    void updateUnauthWarning();
    void updateNetworkWarning();
    void updateWarningsPaneVisibility();
    void updatePresetWarning();
    void updateUrlLabels();
    void updateStatusLabels(const RestServerSettings::Status& status);
    void updateTlsCertificateStatus();
    QString makeHttpUrl() const;
    QString makeHttpsUrl() const;
    QString makeLoopbackCorsAllowlist(bool includeHttp, bool includeHttps) const;
    QString browseForFile(const QString& title, const QString& startDirectory) const;
    QString makeToken() const;
    void refreshTokenTable();
    void updateSelection(int row);
    void applyEditsToSelectedToken();
    void syncEditorsFromSelection();
    void updateTokenVisibility();
    QStringList selectedScopes() const;
    void updateScopeEditors(const QStringList& scopes);
    RestServerToken* selectedToken();

    std::shared_ptr<RestServerSettings> m_settings;
    QList<RestServerToken> m_tokens;
    QString m_fullToken;
    QString m_corsAllowlistToolTip;
    bool m_isTokenVisible{false};
    int m_selectedToken{-1};
};
