#pragma once

#include <QMap>
#include <memory>

#include "preferences/constants.h"
#include "preferences/dialog/dlgpreferencepage.h"
#include "preferences/dialog/ui_dlgprefinterfacedlg.h"
#include "preferences/usersettings.h"
#include "skin/skin.h"

class ControlObject;
class QWidget;

namespace mixxx {
class ScreensaverManager;
namespace skin {
class SkinLoader;
}
} // namespace mixxx

class DlgPrefInterface : public DlgPreferencePage, public Ui::DlgPrefControlsDlg  {
    Q_OBJECT
  public:
    DlgPrefInterface(
            QWidget* parent,
            std::shared_ptr<mixxx::ScreensaverManager> pScreensaverManager,
            std::shared_ptr<mixxx::skin::SkinLoader> pSkinLoader,
            UserSettingsPointer pConfig);
    ~DlgPrefInterface() override = default;

  public slots:
    void slotUpdate() override;
    void slotApply() override;
    void slotResetToDefaults() override;

  private slots:
    void slotSetTooltips();
    void slotSetSkin(int);
    void slotSetScheme(int);
    void slotSetSkinDescription();
    void slotSetSkinPreview();
    void slotUpdateSchemes();

  signals:
    void reloadUserInterface();
    void menuBarAutoHideChanged();
    void tooltipModeChanged(mixxx::preferences::Tooltips tooltipMode);

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
    std::shared_ptr<mixxx::ScreensaverManager> m_pScreensaverManager;
    std::shared_ptr<mixxx::skin::SkinLoader> m_pSkinLoader;

    QMap<QString, mixxx::skin::SkinPointer> m_skins;
    mixxx::skin::SkinPointer m_pSkin;
    QString m_skinNameOnUpdate;
    QString m_colorScheme;
    QString m_colorSchemeOnUpdate;
    QString m_localeOnUpdate;
    mixxx::preferences::MultiSamplingMode m_multiSampling;
    bool m_forceHardwareAcceleration;
    mixxx::preferences::Tooltips m_tooltipMode;
    double m_dScaleFactor;
    double m_minScaleFactor;
    double m_dDevicePixelRatio;
    mixxx::preferences::ScreenSaver m_screensaverMode;
};
