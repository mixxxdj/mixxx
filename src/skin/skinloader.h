#ifndef SKINLOADER_H
#define SKINLOADER_H

#include <QDir>
#include <QList>
#include <QWidget>

#include "control/controlproxy.h"
#include "preferences/usersettings.h"

class KeyboardEventFilter;
class PlayerManager;
class ControllerManager;
class Library;
class VinylControlManager;
class EffectsManager;
class RecordingManager;
class LaunchImage;
class SkinControlObjects;

class SkinLoader : public QObject {
    Q_OBJECT
  public:
    SkinLoader(UserSettingsPointer pConfig);
    virtual ~SkinLoader();

    QWidget* loadConfiguredSkin(QWidget* pParent,
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
    QString pickResizableSkin(QString oldSkin) const;

    UserSettingsPointer m_pConfig;

    QColor m_defaultCueColor;

    ControlProxy m_cpSkinLoaded;
    ControlProxy m_cpFallbackCueColorId;
};

#endif /* SKINLOADER_H */
