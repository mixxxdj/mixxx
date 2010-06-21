// skinloader.cpp
// Created 6/21/2010 by RJ Ryan (rryan@mit.edu)

#include <QString>
#include <QDir>
#include <QtDebug>

#include "skin/skinloader.h"

#include "mixxxview.h"
#include "library/library.h"
#include "playermanager.h"

SkinLoader::SkinLoader(ConfigObject<ConfigValue>* pConfig) :
        m_pConfig(pConfig) {


}


QString SkinLoader::getConfiguredSkinPath() {
    const QString defaultSkin = "outlineNetbook";

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

MixxxView* SkinLoader::loadDefaultSkin(QWidget* pParent,
                                       MixxxKeyboard* pKeyboard,
                                       PlayerManager* pPlayerManager,
                                       Library* pLibrary) {
    return new MixxxView(pParent, pKeyboard, getConfiguredSkinPath(), m_pConfig, pPlayerManager, pLibrary);
}
