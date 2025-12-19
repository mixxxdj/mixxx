#pragma once

#ifdef MIXXX_HAS_HTTP_SERVER

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
    void slotBrowseCertificate();
    void slotBrowseKey();
    void slotAddToken();
    void slotRemoveToken();
    void slotRegenerateToken();
    void slotTokenSelectionChanged();
    void slotTokenDescriptionChanged(const QString& text);
    void slotTokenScopesChanged();
    void slotTokenExpiresChanged(const QDateTime& dateTime);

  private:
    void loadValues(const RestServerSettings::Values& values);
    RestServerSettings::Values gatherValues() const;
    void updateTlsState();
    void updateAuthWarning();
    void updateStatusLabels(const RestServerSettings::Status& status);
    QString browseForFile(const QString& title, const QString& startDirectory) const;
    QString makeToken() const;
    void refreshTokenTable();
    void updateSelection(int row);
    void applyEditsToSelectedToken();
    void syncEditorsFromSelection();
    QStringList selectedScopes() const;
    void updateScopeEditors(const QStringList& scopes);
    RestServerToken* selectedToken();

    std::shared_ptr<RestServerSettings> m_settings;
    QList<RestServerToken> m_tokens;
    int m_selectedToken{-1};
};

#endif // MIXXX_HAS_HTTP_SERVER
