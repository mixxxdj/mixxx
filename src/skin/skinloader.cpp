#include "skin/skinloader.h"

#include <QApplication>
#include <QDir>
#include <QString>
#include <QtDebug>

#include "controllers/controllermanager.h"
#include "effects/effectsmanager.h"
#include "library/library.h"
#include "mixer/playermanager.h"
#include "recording/recordingmanager.h"
#include "skin/legacy/launchimage.h"
#include "skin/legacy/legacyskin.h"
#include "skin/legacy/legacyskinparser.h"
#include "util/debug.h"
#include "util/timer.h"
#include "vinylcontrol/vinylcontrolmanager.h"

using mixxx::skin::SkinPointer;
using mixxx::skin::legacy::LegacySkin;

SkinLoader::SkinLoader(UserSettingsPointer pConfig) :
        m_pConfig(pConfig) {
}

SkinLoader::~SkinLoader() {
    LegacySkinParser::clearSharedGroupStrings();
}

QList<SkinPointer> SkinLoader::getSkins() const {
    const QList<QDir> skinSearchPaths = getSkinSearchPaths();
    QList<SkinPointer> skins;
    for (const QDir& dir : skinSearchPaths) {
        const QList<QFileInfo> fileInfos = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QFileInfo& fileInfo : fileInfos) {
            QDir skinDir(fileInfo.absoluteFilePath());
            if (skinDir.exists(QStringLiteral("skin.xml"))) {
                skins.append(std::make_shared<LegacySkin>(fileInfo));
            }
        }
    }

    return skins;
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

SkinPointer SkinLoader::getSkin(const QString& skinName) const {
    const QList<QDir> skinSearchPaths = getSkinSearchPaths();
    for (QDir dir : skinSearchPaths) {
        if (dir.cd(skinName)) {
            if (dir.exists("skin.xml")) {
                return std::make_shared<LegacySkin>(QFileInfo(dir.absolutePath()));
            }
        }
    }
    return nullptr;
}

SkinPointer SkinLoader::getConfiguredSkin() const {
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

    SkinPointer pSkin = getSkin(configSkin);

    if (pSkin == nullptr || !pSkin->isValid()) {
        const QString defaultSkinName = getDefaultSkinName();
        pSkin = getSkin(defaultSkinName);
        qWarning() << "Could not find the user's configured skin."
                   << "Falling back on the default skin:" << defaultSkinName;
    }
    return pSkin;
}

QString SkinLoader::getDefaultSkinName() const {
    return "LateNight";
}

QWidget* SkinLoader::loadConfiguredSkin(QWidget* pParent,
        QSet<ControlObject*>* pSkinCreatedControls,
        mixxx::CoreServices* pCoreServices) {
    ScopedTimer timer("SkinLoader::loadConfiguredSkin");
    SkinPointer pSkin = getConfiguredSkin();

    // If we don't have a skin then fail.
    VERIFY_OR_DEBUG_ASSERT(pSkin != nullptr && pSkin->isValid()) {
        return nullptr;
    }

    return pSkin->loadSkin(pParent, m_pConfig, pSkinCreatedControls, pCoreServices);
}

LaunchImage* SkinLoader::loadLaunchImage(QWidget* pParent) const {
    SkinPointer pSkin = getConfiguredSkin();
    VERIFY_OR_DEBUG_ASSERT(pSkin != nullptr && pSkin->isValid()) {
        return nullptr;
    }

    LaunchImage* pLaunchImage = pSkin->loadLaunchImage(pParent, m_pConfig);
    if (pLaunchImage == nullptr) {
        // Construct default LaunchImage
        pLaunchImage = new LaunchImage(pParent, QString());
    }

    return pLaunchImage;
}

QString SkinLoader::pickResizableSkin(const QString& oldSkin) const {
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
