#pragma once

#include <QDir>
#include <QList>
#include <QSet>
#include <QWidget>

#include "preferences/usersettings.h"

class KeyboardEventFilter;
class PlayerManager;
class ControllerManager;
class ControlObject;
class Library;
class VinylControlManager;
class EffectsManager;
class RecordingManager;
class LaunchImage;

class SkinLoader {
  public:
    SkinLoader(UserSettingsPointer pConfig);
    virtual ~SkinLoader();

    QWidget* loadConfiguredSkin(QWidget* pParent,
            QSet<ControlObject*>* skinCreatedControls,
            KeyboardEventFilter* pKeyboard,
            PlayerManager* pPlayerManager,
            ControllerManager* pControllerManager,
            Library* pLibrary,
            VinylControlManager* pVCMan,
            EffectsManager* pEffectsManager,
            RecordingManager* pRecordingManager);

    LaunchImage* loadLaunchImage(QWidget* pParent);

    QString getSkinPath(const QString& skinName) const;
    QPixmap getSkinPreview(const QString& skinName, const QString& schemeName) const;
    QString getConfiguredSkinPath() const;
    QString getDefaultSkinName() const;
    QList<QDir> getSkinSearchPaths() const;

  private:
    QString pickResizableSkin(const QString& oldSkin) const;

    UserSettingsPointer m_pConfig;
};
