#ifndef SKINLOADER_H
#define SKINLOADER_H

#include <QWidget>
#include <QList>
#include <QDir>

#include "configobject.h"

class MixxxKeyboard;
class PlayerManager;
class ControllerManager;
class Library;
class VinylControlManager;
class EffectsManager;
class LaunchImage;

class SkinLoader {
  public:
    SkinLoader(ConfigObject<ConfigValue>* pConfig);
    virtual ~SkinLoader();

    QWidget* loadDefaultSkin(QWidget* pParent,
                             MixxxKeyboard* pKeyboard,
                             PlayerManager* pPlayerManager,
                             ControllerManager* pControllerManager,
                             Library* pLibrary,
                             VinylControlManager* pVCMan,
                             EffectsManager* pEffectsManager);

    LaunchImage* loadLaunchImage(QWidget* pParent);

    QString getSkinPath();
    QList<QDir> getSkinSearchPaths();

  private:
    QString getConfiguredSkinPath();
    QString getDefaultSkinName() const;
    QString getDefaultSkinPath();
    QString pickResizableSkin(QString oldSkin);

    ConfigObject<ConfigValue>* m_pConfig;
};


#endif /* SKINLOADER_H */
