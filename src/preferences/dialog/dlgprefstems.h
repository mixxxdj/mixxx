#pragma once

#include <QWidget>

#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/usersettings.h"

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;

/// Preferences page for stem generation, see StemGenerator.
///
/// The separation itself runs in an external worker process, so this page is
/// mostly about showing what the generator found out about the environment and
/// what is in the queue, plus the settings that are handed over to the worker.
class DlgPrefStems : public DlgPreferencePage {
    Q_OBJECT
  public:
    DlgPrefStems(QWidget* pParent, UserSettingsPointer pConfig);
    ~DlgPrefStems() override;

  public slots:
    void slotUpdate() override;
    void slotApply() override;
    void slotResetToDefaults() override;
    void slotShow() override;

  private slots:
    void slotQueueChanged();
    void slotCancelQueue();
    void slotClearFinished();
    void slotOpenStemFolder();

  private:
    void refreshEnvironment();
    void refreshQueue();
    void setControlsEnabled(bool enabled);

    UserSettingsPointer m_pConfig;

    QLabel* m_pEnvironmentLabel{nullptr};
    QLabel* m_pErrorsLabel{nullptr};
    QCheckBox* m_pEnabledCheckBox{nullptr};
    QLineEdit* m_pPythonPathEdit{nullptr};
    QLineEdit* m_pCacheDirectoryEdit{nullptr};
    QLineEdit* m_pFfmpegPathEdit{nullptr};
    QComboBox* m_pCodecComboBox{nullptr};
    QComboBox* m_pModelComboBox{nullptr};
    QComboBox* m_pProviderComboBox{nullptr};
    QCheckBox* m_pWriteNextToTrackCheckBox{nullptr};
    QCheckBox* m_pPauseDuringPlaybackCheckBox{nullptr};
    QPushButton* m_pProbeButton{nullptr};
    QPushButton* m_pCancelButton{nullptr};
    QPushButton* m_pClearButton{nullptr};
    QPushButton* m_pOpenFolderButton{nullptr};
    QTableWidget* m_pQueueTable{nullptr};
};
