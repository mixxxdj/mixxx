#pragma once

#include <QSpinBox>
#include <QWidget>
#include <memory>

#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefvinyldlg.h"
#include "preferences/usersettings.h"
#include "vinylcontrol/vinylcontrolsignalwidget.h"

class ControlProxy;
class VinylControlManager;

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
    void VinylTypeSlotApply();
    void slotVinylGainApply();
    void slotUpdateVinylGain();

  private slots:
    void slotNumDecksChanged(double);
    void slotVinylType1Changed(const QString&);
    void slotVinylType2Changed(const QString&);
    void slotVinylType3Changed(const QString&);
    void slotVinylType4Changed(const QString&);

  private:
    void setDeckWidgetsVisible(int deck, bool visible);
    void setDeck1WidgetsVisible(bool visible);
    void setDeck2WidgetsVisible(bool visible);
    void setDeck3WidgetsVisible(bool visible);
    void setDeck4WidgetsVisible(bool visible);
    void verifyAndSaveLeadInTime(QSpinBox* widget, const QString& group, const QString& vinyl_type);
    int getDefaultLeadIn(const QString& vinyl_type) const;

    QList<VinylControlSignalWidget*> m_signalWidgets;

    std::shared_ptr<VinylControlManager> m_pVCManager;
    UserSettingsPointer config;
    QList<ControlProxy*> m_COSpeeds;
    ControlProxy* m_pNumDecks;
};
