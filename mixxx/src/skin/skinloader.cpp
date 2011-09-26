// skinloader.cpp
// Created 6/21/2010 by RJ Ryan (rryan@mit.edu)

#include <QString>
#include <QDir>
#include <QtDebug>

#include "vinylcontrol/vinylcontrolmanager.h"
#include "skin/skinloader.h"
#include "skin/legacyskinparser.h"

#include "library/library.h"
#include "playermanager.h"

SkinLoader::SkinLoader(ConfigObject<ConfigValue>* pConfig) :
        m_pConfig(pConfig) {


}

SkinLoader::~SkinLoader() {
    LegacySkinParser::freeChannelStrings();
}

QString SkinLoader::getConfiguredSkinPath() {
    const QString defaultSkin = "Deere1280x800-WXGA";//XXX: App Store //"Outline1024x600-Netbook";

    QString qSkinPath = m_pConfig->getConfigPath();
    qSkinPath.append("skins/");

    QDir skinPath(qSkinPath);

    if (skinPath.exists()) {
        // Is the skin listed in the config database there? If not, use default skin.

        QString configSkin = m_pConfig->getValueString(ConfigKey("[Config]","Skin"));
        QString qThisSkin = qSkinPath + configSkin;
        QDir thisSkin(qThisSkin);

        if (configSkin.length() > 0 && thisSkin.exists()) {
            qSkinPath = qThisSkin;
        } else {
            m_pConfig->set(ConfigKey("[Config]","Skin"), ConfigValue(defaultSkin));
            qSkinPath.append(defaultSkin);
        }
    } else {
        qCritical() << "Skin directory does not exist:" << qSkinPath;
    }

    return qSkinPath;
}

QWidget* SkinLoader::loadDefaultSkin(QWidget* pParent,
                                       MixxxKeyboard* pKeyboard,
                                       PlayerManager* pPlayerManager,
                                       Library* pLibrary,
                                       VinylControlManager* pVCMan) {
    QString skinPath = getConfiguredSkinPath();

    LegacySkinParser legacy(m_pConfig, pKeyboard, pPlayerManager, pLibrary, pVCMan);
    return legacy.parseSkin(skinPath, pParent);
}
