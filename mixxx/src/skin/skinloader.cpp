// skinloader.cpp
// Created 6/21/2010 by RJ Ryan (rryan@mit.edu)

#include <QString>
#include <QDir>
#include <QtDebug>

#include "vinylcontrol/vinylcontrolmanager.h"
#include "skin/skinloader.h"
#include "skin/legacyskinparser.h"
#include "controllers/controllermanager.h"
#include "library/library.h"
#include "playermanager.h"

SkinLoader::SkinLoader(ConfigObject<ConfigValue>* pConfig) :
        m_pConfig(pConfig) {


}

SkinLoader::~SkinLoader() {
    LegacySkinParser::freeChannelStrings();
}

QString SkinLoader::getConfiguredSkinPath() {

    QString qSkinPath = m_pConfig->getConfigPath();
    qSkinPath.append("skins/");

    QString configSkin = m_pConfig->getValueString(ConfigKey("[Config]","Skin"));
    QString qThisSkin = qSkinPath + configSkin;
    QDir thisSkin(qThisSkin);

    if (configSkin.length() > 0 && thisSkin.exists()) {
        qSkinPath = qThisSkin;
    } else {
        // Fall back to default skin
        QString defaultSkin;
        QRect screenGeo = QApplication::desktop()->screenGeometry();
        if (screenGeo.width() >= 1280 && screenGeo.height() >= 800) {
            defaultSkin = "Deere1280x800-WXGA";
        }
        else if (screenGeo.width() >= 1024 && screenGeo.height() >= 600) {
            defaultSkin = "ShadeDark1024x600-Netbook";
        }
        else {
            defaultSkin = "Outline800x480-WVGA"; // Mixxx's smallest Skin
        }
        qSkinPath.append(defaultSkin);
    }

    QDir skinPath(qSkinPath);
    if (!skinPath.exists()) {
        qCritical() << "Skin directory does not exist:" << qSkinPath;
    }

    return qSkinPath;
}

QWidget* SkinLoader::loadDefaultSkin(QWidget* pParent,
                                     MixxxKeyboard* pKeyboard,
                                     PlayerManager* pPlayerManager,
                                     ControllerManager* pControllerManager,
                                     Library* pLibrary,
                                     VinylControlManager* pVCMan) {
    QString skinPath = getConfiguredSkinPath();

    LegacySkinParser legacy(m_pConfig, pKeyboard, pPlayerManager, pControllerManager, pLibrary, pVCMan);
    return legacy.parseSkin(skinPath, pParent);
}
