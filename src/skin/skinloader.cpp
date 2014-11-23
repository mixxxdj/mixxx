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

QString SkinLoader::getConfiguredSkinPath() {

    QString qSkinPath = m_pConfig->getResourcePath();
    qSkinPath.append("skins/");

    QString configSkin = m_pConfig->getValueString(ConfigKey("[Config]","Skin"));
    QString qThisSkin = qSkinPath + configSkin;
    QDir thisSkin(qThisSkin);

    if (configSkin.length() > 0 && thisSkin.exists()) {
        qSkinPath = qThisSkin;
    } else {
        // try developer path
        qSkinPath = m_pConfig->getResourcePath();
        qSkinPath.append("developer_skins/");
        qThisSkin = qSkinPath + configSkin;
        thisSkin = qThisSkin;
        if (configSkin.length() > 0 && thisSkin.exists()) {
            qSkinPath = qThisSkin;
        } else {
            // Fall back to default skin
            QString defaultSkin;
            QRect screenGeo = QApplication::desktop()->screenGeometry();
            if (screenGeo.width() >= 1280 && screenGeo.height() >= 800) {
                defaultSkin = "LateNight";
            }
            else if (screenGeo.width() >= 1024 && screenGeo.height() >= 600) {
                defaultSkin = "ShadeDark1024x600-Netbook";
            }
            else {
                defaultSkin = "Outline800x480-WVGA"; // Mixxx's smallest Skin
            }
            qSkinPath = m_pConfig->getResourcePath();
            qSkinPath.append("skins/");
            qSkinPath.append(defaultSkin);
        }
    }

    QDir skinPath(qSkinPath);
    if (!skinPath.exists()) {
        reportCriticalErrorAndQuit("Skin directory does not exist: " + qSkinPath);
    }

    return qSkinPath;
}

QWidget* SkinLoader::loadDefaultSkin(QWidget* pParent,
                                     MixxxKeyboard* pKeyboard,
                                     PlayerManager* pPlayerManager,
                                     ControllerManager* pControllerManager,
                                     Library* pLibrary,
                                     VinylControlManager* pVCMan,
                                     EffectsManager* pEffectsManager) {
    QString skinPath = getConfiguredSkinPath();
    LegacySkinParser legacy(m_pConfig, pKeyboard, pPlayerManager,
                            pControllerManager, pLibrary, pVCMan,
                            pEffectsManager);
    return legacy.parseSkin(skinPath, pParent);
}
