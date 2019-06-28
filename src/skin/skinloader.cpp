// skinloader.cpp
// Created 6/21/2010 by RJ Ryan (rryan@mit.edu)

#include "skin/skinloader.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
#include <QString>
#include <QtDebug>

#include "controllers/controllermanager.h"
#include "effects/effectsmanager.h"
#include "library/library.h"
#include "mixer/playermanager.h"
#include "recording/recordingmanager.h"
#include "skin/launchimage.h"
#include "skin/legacyskinparser.h"
#include "skin/skincontext.h"
#include "util/debug.h"
#include "util/timer.h"
#include "vinylcontrol/vinylcontrolmanager.h"

SkinLoader::SkinLoader(UserSettingsPointer pConfig)
        : m_pConfig(pConfig),
          m_defaultCueColor(),
          m_coSkinLoaded(ConfigKey("[Skin]", "reloaded")),
          m_coFallbackCueColorId(ConfigKey("[Skin]", "fallback_cue_color_id")) {
}

SkinLoader::~SkinLoader() {
    LegacySkinParser::freeChannelStrings();
}

QList<QDir> SkinLoader::getSkinSearchPaths() const {
    QList<QDir> searchPaths;

    // Add user skin path to search paths
    QDir userSkinsPath(m_pConfig->getSettingsPath());
    if (userSkinsPath.cd("skins")) {
        searchPaths.append(userSkinsPath);
    }

    // If we can't find the skins folder then we can't load a skin at all. This
    // is a critical error in the user's Mixxx installation.
    QDir skinsPath(m_pConfig->getResourcePath());
    if (!skinsPath.cd("skins")) {
        reportCriticalErrorAndQuit("Skin directory does not exist: " +
                                   skinsPath.absoluteFilePath("skins"));
    }
    searchPaths.append(skinsPath);

    return searchPaths;
}

QString SkinLoader::getSkinPath(const QString& skinName) const {
    for (QDir dir : getSkinSearchPaths()) {
        if (dir.cd(skinName)) {
            return dir.absolutePath();
        }
    }
    return QString();
}

QPixmap SkinLoader::getSkinPreview(const QString& skinName, const QString& schemeName) const {
    qDebug() << "schemeName =" << schemeName;
    QPixmap preview;
    if (!schemeName.isEmpty()) {
        QString schemeNameUnformatted = schemeName;
        QString schemeNameFormatted = schemeNameUnformatted.replace(" ","");
        preview.load(getSkinPath(skinName) + "/skin_preview_" + schemeNameFormatted + ".png");
    } else {
        preview.load(getSkinPath(skinName) + "/skin_preview.png");
    }
    if (!preview.isNull()){
        return preview;
    }
    preview.load(":/images/skin_preview_placeholder.png");
    return preview;
}

QString SkinLoader::getConfiguredSkinPath() const {
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
    }

    QString skinPath = getSkinPath(configSkin);

    if (skinPath.isEmpty()) {
        skinPath = getSkinPath(getDefaultSkinName());
        qDebug() << "Could not find the user's configured skin."
                 << "Falling back on the default skin:" << skinPath;
    }
    return skinPath;
}

QString SkinLoader::getDefaultSkinName() const {
    return "Deere";
}

QWidget* SkinLoader::loadConfiguredSkin(QWidget* pParent,
                                        KeyboardEventFilter* pKeyboard,
                                        PlayerManager* pPlayerManager,
                                        ControllerManager* pControllerManager,
                                        Library* pLibrary,
                                        VinylControlManager* pVCMan,
                                        EffectsManager* pEffectsManager,
                                        RecordingManager* pRecordingManager) {
    ScopedTimer timer("SkinLoader::loadConfiguredSkin");
    QString skinPath = getConfiguredSkinPath();

    // If we don't have a skin path then fail.
    if (skinPath.isEmpty()) {
        return NULL;
    }

    LegacySkinParser parser(m_pConfig, pKeyboard, pPlayerManager, pControllerManager, pLibrary, pVCMan, pEffectsManager, pRecordingManager);
    QWidget* skin = parser.parseSkin(skinPath, pParent);

    QColor controllersDefaultCueColor =
            parser.m_pContext->getControllersDefaultCueColor();
    PredefinedColorPointer controllersFallbackColor =
            parser.m_pContext->getControllersFallbackCueColor();

    pControllerManager->setDefaultCueColor(controllersDefaultCueColor);
    m_coFallbackCueColorId.set(controllersFallbackColor->m_iId);
    m_coSkinLoaded.set(1);
    return skin;
}

LaunchImage* SkinLoader::loadLaunchImage(QWidget* pParent) {
    QString skinPath = getConfiguredSkinPath();
    LegacySkinParser parser(m_pConfig);
    LaunchImage* pLaunchImage = parser.parseLaunchImage(skinPath, pParent);
    if (pLaunchImage == nullptr) {
        // Construct default LaunchImage
        pLaunchImage = new LaunchImage(pParent, QString());
    }
    return pLaunchImage;
}

QString SkinLoader::pickResizableSkin(QString oldSkin) const {
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
