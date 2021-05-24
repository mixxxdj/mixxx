#pragma once

#include <QDir>
#include <QList>
#include <QSet>
#include <QWidget>

#include "preferences/usersettings.h"
#include "skin/skin.h"

class ControlObject;
class LaunchImage;
namespace mixxx {
class CoreServices;
}

class SkinLoader {
  public:
    SkinLoader(UserSettingsPointer pConfig);
    virtual ~SkinLoader();

    QWidget* loadConfiguredSkin(QWidget* pParent,
            QSet<ControlObject*>* skinCreatedControls,
            mixxx::CoreServices* pCoreServices);

    LaunchImage* loadLaunchImage(QWidget* pParent);

    mixxx::skin::SkinPointer getSkin(const QString& skinName) const;
    mixxx::skin::SkinPointer getConfiguredSkin() const;
    QString getDefaultSkinName() const;
    QList<QDir> getSkinSearchPaths() const;
    QList<mixxx::skin::SkinPointer> getSkins() const;

  private:
    QString pickResizableSkin(const QString& oldSkin) const;

    UserSettingsPointer m_pConfig;
};
