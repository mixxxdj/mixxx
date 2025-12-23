#pragma once

#include <memory>

#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefvinyldlg.h"
#include "preferences/usersettings.h"

class ControlProxy;
class PollingControlProxy;
class VinylControlManager;
class QSpinBox;
class QWidget;
class VinylControlSignalWidget;

class DlgPrefVinyl : public DlgPreferencePage, Ui::DlgPrefVinylDlg  {
    Q_OBJECT
  public:
    DlgPrefVinyl(
            QWidget* pParent,
            std::shared_ptr<VinylControlManager> m_pVCMan,
            UserSettingsPointer _config);
    virtual ~DlgPrefVinyl();

    QUrl helpUrl() const override;

  public slots:
    void slotUpdate() override;
    void slotApply() override;
    void slotResetToDefaults() override;

    void slotHide() override;
    void slotShow() override;
    void slotVinylGainApply();
    void slotUpdateVinylGain();

  private slots:
    void slotNumDecksChanged(double);
    void slotVinylTypeChanged(const QString&);

  private:
    void setDeckWidgetsVisible(int deck, bool visible);
    void verifyAndSaveLeadInTime(int deck, const QString& group);
    int getDefaultLeadIn(const QString& vinyl_type) const;

    QList<QLabel*> m_vcLabels;
    QList<QComboBox*> m_vcTypeBoxes;
    QList<QComboBox*> m_vcSpeedBoxes;
    QList<QComboBox*> m_vcPitchEstimatorBoxes;
    QList<QSpinBox*> m_vcLeadInBoxes;
    QList<VinylControlSignalWidget*> m_signalWidgets;

    std::shared_ptr<VinylControlManager> m_pVCManager;
    UserSettingsPointer config;
    QList<PollingControlProxy*> m_COSpeeds;
    ControlProxy* m_pNumDecks;
};
