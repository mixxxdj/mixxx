#ifndef SKINLOADER_H
#define SKINLOADER_H

#include <QWidget>
#include <QList>
#include <QDir>

#include "controllers/keyboard/keyboardcontrollerpreset.h"
#include "preferences/usersettings.h"
#include "skin/tooltipupdater.h"

class KeyboardEventFilter;
class PlayerManager;
class ControllerManager;
class Library;
class VinylControlManager;
class EffectsManager;
class LaunchImage;

class SkinLoader : public QObject {
    Q_OBJECT
  public:
    SkinLoader(UserSettingsPointer pConfig);
    virtual ~SkinLoader();

    QWidget* loadDefaultSkin(QWidget* pParent,
                             KeyboardEventFilter* pKeyboard,
                             PlayerManager* pPlayerManager,
                             ControllerManager* pControllerManager,
                             Library* pLibrary,
                             VinylControlManager* pVCMan,
                             EffectsManager* pEffectsManager);

    LaunchImage* loadLaunchImage(QWidget* pParent);

    QString getSkinPath();
    QList<QDir> getSkinSearchPaths();
    inline const TooltipShortcutUpdater *getTooltipUpdater() {
        return &m_TooltipUpdater;
    };

  private:
    QString getConfiguredSkinPath();
    QString getDefaultSkinName() const;
    QString getDefaultSkinPath();
    QString pickResizableSkin(QString oldSkin);
    TooltipShortcutUpdater m_TooltipUpdater;
    UserSettingsPointer m_pConfig;
};


#endif /* SKINLOADER_H */
