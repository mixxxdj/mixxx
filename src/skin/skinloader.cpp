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
#include "skin/launchimage.h"

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
    QString configSkin = m_pConfig->getValueString(ConfigKey("[Config]", "ResizableSkin"));

    // If we don't have a skin defined, we might be migrating from 1.11 and
    // should pick the closest-possible skin.
    if (configSkin.isEmpty()) {
        QString oldSkin = m_pConfig->getValueString(ConfigKey("[Config]", "Skin"));
        if (!oldSkin.isEmpty()) {
            configSkin = pickResizableSkin(oldSkin);
        }
        // If the old skin was empty or we couldn't guess a skin, go with the
        // default.
        if (configSkin.isEmpty()) {
            configSkin = getDefaultSkinName();
        }
        m_pConfig->set(ConfigKey("[Config]", "ResizableSkin"),
                       ConfigValue(configSkin));
    }

    QList<QDir> skinSearchPaths = getSkinSearchPaths();
    foreach (QDir dir, skinSearchPaths) {
        if (dir.cd(configSkin)) {
            return dir.absolutePath();
        }
    }

    return QString();
}

QString SkinLoader::getDefaultSkinName() const {
    QRect screenGeo = QApplication::desktop()->screenGeometry();
    if (screenGeo.width() >= 1280 && screenGeo.height() >= 800) {
        return "LateNight";
    } else {
        return "Shade";
    }
}

QString SkinLoader::getDefaultSkinPath() {
    // Fall back to default skin.
    QString defaultSkin = getDefaultSkinName();

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

LaunchImage* SkinLoader::loadLaunchImage(QWidget* pParent) {
    QString skinPath = getSkinPath();
    LegacySkinParser parser;
    LaunchImage* pLaunchImage = parser.parseLaunchImage(skinPath, pParent);
    if (pLaunchImage == NULL) {
        // Construct default LaunchImage
        pLaunchImage = new LaunchImage(pParent, QString());
    }
    return pLaunchImage;
}

QString SkinLoader::pickResizableSkin(QString oldSkin) {
    if (oldSkin.contains("latenight", Qt::CaseInsensitive)) {
        return "LateNight";
    }
    if (oldSkin.contains("deere", Qt::CaseInsensitive)) {
        return "Deere";
    }
    if (oldSkin.contains("shade", Qt::CaseInsensitive)) {
        return "Shade";
    }
    return QString();
}
