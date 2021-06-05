#pragma once

#include <QMap>
#include <QWidget>
#include <memory>
#include <optional>

#include "preferences/constants.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefinterfacedlg.h"
#include "preferences/usersettings.h"
#include "skin/skin.h"

class ControlProxy;
class ControlPotmeter;
class PlayerManager;
class MixxxMainWindow;
class ControlObject;

namespace mixxx {
namespace skin {
class SkinLoader;
}
} // namespace mixxx

class DlgPrefInterface : public DlgPreferencePage, public Ui::DlgPrefControlsDlg  {
    Q_OBJECT
  public:
    DlgPrefInterface(
            QWidget* parent,
            MixxxMainWindow* mixxx,
            std::shared_ptr<mixxx::skin::SkinLoader> pSkinLoader,
            UserSettingsPointer pConfig);
    ~DlgPrefInterface() override = default;

  public slots:
    void slotUpdate() override;
    void slotApply() override;
    void slotResetToDefaults() override;

  private slots:
    void slotSetTooltips();
    void slotSetSkinDescription();
    void slotSetSkin(int);
    void slotSetScheme(int);
    void slotUpdateSchemes();
    void slotSetScaleFactor(double newValue);
    void slotSetScaleFactorAuto(bool checked);

  private:
    void notifyRebootNecessary();
    void loadTooltipPreferenceFromConfig();

    // Because the CueDefault list is out of order, we have to set the combo
    // box using the user data, not the index.  Returns the index of the item
    // that has the corresponding userData. If the userdata is not in the list,
    // returns zero.
    int cueDefaultIndexByData(int userData) const;
    QScreen* getScreen() const;

    UserSettingsPointer m_pConfig;
    ControlObject* m_pControlTrackTimeDisplay;
    MixxxMainWindow *m_mixxx;
    std::shared_ptr<mixxx::skin::SkinLoader> m_pSkinLoader;

    QMap<QString, mixxx::skin::SkinPointer> m_skins;
    mixxx::skin::SkinPointer m_pSkin;
    QString m_skinNameOnUpdate;
    QString m_colorScheme;
    QString m_localeOnUpdate;
    mixxx::TooltipsPreference m_tooltipMode;
    double m_dScaleFactorAuto;
    bool m_bUseAutoScaleFactor;
    double m_dScaleFactor;
    double m_dDevicePixelRatio;
    bool m_bStartWithFullScreen;
    mixxx::ScreenSaverPreference m_screensaverMode;

    bool m_bRebootMixxxView;
};
