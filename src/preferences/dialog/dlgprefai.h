#pragma once

#include <QList>
#include <QString>

#include "ai/aimixplanner.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefai.h"
#include "preferences/usersettings.h"

/// Preferences page for the optional AI set planner.
///
/// The API key is stored in the system keychain instead of mixxx.cfg, see
/// mixxx::ai::storeApiKey().
class DlgPrefAi : public DlgPreferencePage, public Ui::DlgPrefAiDlg {
    Q_OBJECT
  public:
    DlgPrefAi(QWidget* pParent, UserSettingsPointer pConfig);
    ~DlgPrefAi() override;

  public slots:
    void slotUpdate() override;
    void slotApply() override;
    void slotResetToDefaults() override;

  private slots:
    void slotProviderChanged(int index);
    void slotTestConnection();
    void slotProbeSucceeded(const QList<int>& trackOrder);
    void slotProbeFailed(const QString& errorMessage);

  private:
    void setStatusMessage(const QString& message, bool isError);
    void loadProviderDefaults(mixxx::ai::AiMixSettings::Provider provider);
    /// Keychain access is asynchronous on purpose: a blocking read runs a nested
    /// event loop, which can destroy this page while the call is still running.
    void requestStoredApiKey(const QString& providerId);
    void writeStoredApiKey(const QString& providerId, const QString& apiKey);

    UserSettingsPointer m_pConfig;
    mixxx::ai::AiMixPlanner* const m_pPlanner;

    /// Set while slotUpdate() fills the widgets, to keep the provider selection
    /// from overwriting the values just loaded from the settings.
    bool m_loading = false;
    /// True while the "test connection" probe is in flight.
    bool m_testingConnection = false;
};