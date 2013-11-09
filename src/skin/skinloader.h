#ifndef SKINLOADER_H
#define SKINLOADER_H

#include <QWidget>

#include "configobject.h"

class MixxxKeyboard;
class PlayerManager;
class ControllerManager;
class Library;
class MixxxView;
class VinylControlManager;
class TrackCollection;

class SkinLoader {
  public:
    SkinLoader(ConfigObject<ConfigValue>* pConfig);
    ~SkinLoader();
    QWidget* loadDefaultSkin(QWidget* pParent,
                             MixxxKeyboard* pKeyboard,
                             PlayerManager* pPlayerManager,
                             ControllerManager* pControllerManager,
                             Library* pLibrary,
                             VinylControlManager* pVCMan,
                             TrackCollection* pTrackCollection);

    QString getConfiguredSkinPath();

  private:
    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
};


#endif /* SKINLOADER_H */

