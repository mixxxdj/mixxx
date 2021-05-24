#pragma once

#include <QDir>
#include <QList>
#include <QSet>
#include <QWidget>

#include "preferences/usersettings.h"
#include "skin/skin.h"

class SkinLoader {
  public:
    SkinLoader(UserSettingsPointer pConfig);
    virtual ~SkinLoader();

    QWidget* loadConfiguredSkin(QWidget* pParent,
            QSet<ControlObject*>* skinCreatedControls,
            mixxx::CoreServices* pCoreServices);

    LaunchImage* loadLaunchImage(QWidget* pParent) const;

    mixxx::skin::SkinPointer getSkin(const QString& skinName) const;
    mixxx::skin::SkinPointer getConfiguredSkin() const;
    QString getDefaultSkinName() const;
    QList<QDir> getSkinSearchPaths() const;
    QList<mixxx::skin::SkinPointer> getSkins() const;

  private:
    QString pickResizableSkin(const QString& oldSkin) const;
    mixxx::skin::SkinPointer skinFromDirectory(const QDir& dir) const;

    UserSettingsPointer m_pConfig;
};
