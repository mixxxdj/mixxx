#pragma once

#include <QDir>
#include <QList>
#include <QSet>
#include <QWidget>

#include "preferences/usersettings.h"
#include "skin/skin.h"

namespace mixxx {
namespace skin {

class SkinLoader {
  public:
    SkinLoader(UserSettingsPointer pConfig);
    virtual ~SkinLoader();

    QWidget* loadConfiguredSkin(QWidget* pParent,
            QSet<ControlObject*>* skinCreatedControls,
            mixxx::CoreServices* pCoreServices);

    LaunchImage* loadLaunchImage(QWidget* pParent) const;

    SkinPointer getSkin(const QString& skinName) const;
    SkinPointer getConfiguredSkin() const;
    QString getDefaultSkinName() const;
    QList<QDir> getSkinSearchPaths() const;
    QList<SkinPointer> getSkins() const;

  private:
    QString pickResizableSkin(const QString& oldSkin) const;
    SkinPointer skinFromDirectory(const QDir& dir) const;

    UserSettingsPointer m_pConfig;
};

} // namespace skin
} // namespace mixxx
