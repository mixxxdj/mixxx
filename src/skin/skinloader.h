#ifndef SKINLOADER_H
#define SKINLOADER_H

#include <QWidget>
#include <QList>
#include <QDir>

#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "preferences/usersettings.h"

class KeyboardEventFilter;
class PlayerManager;
class ControllerManager;
class Library;
class VinylControlManager;
class EffectsManager;
class RecordingManager;
class LaunchImage;

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

    ControlPushButton m_coSkinLoaded;
    ControlObject m_coFallbackCueColorId;
};

#endif /* SKINLOADER_H */
