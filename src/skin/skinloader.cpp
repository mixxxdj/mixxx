// skinloader.cpp
// Created 6/21/2010 by RJ Ryan (rryan@mit.edu)

#include "skin/skinloader.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
#include <QString>
#include <QtDebug>

#include "vinylcontrol/vinylcontrolmanager.h"
#include "skin/legacyskinparser.h"
#include "controllers/controllermanager.h"
#include "library/library.h"
#include "effects/effectsmanager.h"
#include "playermanager.h"
#include "util/debug.h"

SkinLoader::SkinLoader(ConfigObject<ConfigValue>* pConfig) :
        m_pConfig(pConfig) {
}

SkinLoader::~SkinLoader() {
    LegacySkinParser::freeChannelStrings();
}

QList<QDir> SkinLoader::getSkinSearchPaths() {
    QList<QDir> searchPaths;
    // If we can't find the skins folder then we can't load a skin at all. This
    // is a critical error in the user's Mixxx installation.
    QDir skinsPath(m_pConfig->getResourcePath());
    if (!skinsPath.cd("skins")) {
        reportCriticalErrorAndQuit("Skin directory does not exist: " +
                                   skinsPath.absoluteFilePath("skins"));
    }
    searchPaths.append(skinsPath);

    QDir developerSkinsPath(m_pConfig->getResourcePath());
    if (developerSkinsPath.cd("developer_skins")) {
        searchPaths.append(developerSkinsPath);
    }

    return searchPaths;
}

QString SkinLoader::getConfiguredSkinPath() {
    QString configSkin = m_pConfig->getValueString(ConfigKey("[Config]", "Skin"));
    if (configSkin.isEmpty()) {
        return QString();
    }

    QList<QDir> skinSearchPaths = getSkinSearchPaths();
    foreach (QDir dir, skinSearchPaths) {
        if (dir.cd(configSkin)) {
            return dir.absolutePath();
        }
    }

    return QString();
}

QString SkinLoader::getDefaultSkinPath() {
    // Fall back to default skin.
    QString defaultSkin;
    QRect screenGeo = QApplication::desktop()->screenGeometry();
    if (screenGeo.width() >= 1280 && screenGeo.height() >= 800) {
        defaultSkin = "LateNight";
    } else {
        defaultSkin = "Shade";
    }

    QList<QDir> skinSearchPaths = getSkinSearchPaths();
    foreach (QDir dir, skinSearchPaths) {
        if (dir.cd(defaultSkin)) {
            return dir.absolutePath();
        }
    }

    return QString();
}

QString SkinLoader::getSkinPath() {
    QString skinPath = getConfiguredSkinPath();

    if (skinPath.isEmpty()) {
        skinPath = getDefaultSkinPath();
        qDebug() << "Could not find the user's configured skin."
                 << "Falling back on the default skin:" << skinPath;
    }
    return skinPath;
}

QWidget* SkinLoader::loadDefaultSkin(QWidget* pParent,
                                     MixxxKeyboard* pKeyboard,
                                     PlayerManager* pPlayerManager,
                                     ControllerManager* pControllerManager,
                                     Library* pLibrary,
                                     VinylControlManager* pVCMan,
                                     EffectsManager* pEffectsManager) {
    QString skinPath = getSkinPath();

    // If we don't have a skin path then fail.
    if (skinPath.isEmpty()) {
        return NULL;
    }

    LegacySkinParser legacy(m_pConfig, pKeyboard, pPlayerManager,
                            pControllerManager, pLibrary, pVCMan,
                            pEffectsManager);
    return legacy.parseSkin(skinPath, pParent);
}
