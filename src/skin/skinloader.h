#ifndef SKINLOADER_H
#define SKINLOADER_H

#include <QWidget>
#include <QList>
#include <QDir>

#include "preferences/usersettings.h"

class KeyboardEventFilter;
class PlayerManager;
class ControllerManager;
class Library;
class VinylControlManager;
class EffectsManager;
class RecordingManager;
class LaunchImage;

class SkinLoader {
  public:
    SkinLoader(UserSettingsPointer pConfig);
    virtual ~SkinLoader();

    QWidget* loadDefaultSkin(QWidget* pParent,
                             KeyboardEventFilter* pKeyboard,
                             PlayerManager* pPlayerManager,
                             ControllerManager* pControllerManager,
                             Library* pLibrary,
                             VinylControlManager* pVCMan,
                             EffectsManager* pEffectsManager,
                             RecordingManager* pRecordingManager);

    LaunchImage* loadLaunchImage(QWidget* pParent);

    QString getSkinPath();
    QList<QDir> getSkinSearchPaths();

  private:
    QString getConfiguredSkinPath();
    QString getDefaultSkinName() const;
    QString getDefaultSkinPath();
    QString pickResizableSkin(QString oldSkin);

    UserSettingsPointer m_pConfig;
};


#endif /* SKINLOADER_H */
