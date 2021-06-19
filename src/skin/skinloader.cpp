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
#include "skin/qml/qmlskin.h"
#include "util/debug.h"
#include "util/timer.h"
#include "vinylcontrol/vinylcontrolmanager.h"

namespace mixxx {
namespace skin {

using legacy::LegacySkin;
using qml::QmlSkin;

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
            SkinPointer pSkin = skinFromDirectory(skinDir);
            if (pSkin) {
                VERIFY_OR_DEBUG_ASSERT(pSkin->isValid()) {
                    continue;
                }
                skins.append(pSkin);
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
            SkinPointer pSkin = skinFromDirectory(dir);
            if (pSkin) {
                VERIFY_OR_DEBUG_ASSERT(pSkin->isValid()) {
                    continue;
                }
                return pSkin;
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
    }

    // Pick default skin otherwise
    if (configSkin.isEmpty()) {
        configSkin = getDefaultSkinName();
    }

    // Try to load the desired skin
    DEBUG_ASSERT(!configSkin.isEmpty());
    SkinPointer pSkin = getSkin(configSkin);
    if (pSkin && pSkin->isValid()) {
        qInfo() << "Loaded skin" << configSkin;
        return pSkin;
    }
    qWarning() << "Failed to load skin" << configSkin;

    // Fallback to default skin as last resort
    const QString defaultSkinName = getDefaultSkinName();
    DEBUG_ASSERT(!defaultSkinName.isEmpty());
    pSkin = getSkin(defaultSkinName);
    VERIFY_OR_DEBUG_ASSERT(pSkin && pSkin->isValid()) {
        qWarning() << "Failed to load default skin" << defaultSkinName;
        return nullptr;
    }
    qInfo() << "Loaded default skin" << defaultSkinName;
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

    // If we don't have a skin then fail. This makes sense here, because the
    // method above already tried to fall back to the default skin if the
    // configured one is not available. If `pSkin` is nullptr, we both the
    // configured and the default skin were not found, so there is nothing we
    // can do.
    VERIFY_OR_DEBUG_ASSERT(pSkin != nullptr && pSkin->isValid()) {
        return nullptr;
    }

    QWidget* pLoadedSkin = pSkin->loadSkin(pParent, m_pConfig, pSkinCreatedControls, pCoreServices);

    // If the skin exists but failed to load, try to fall back to the default skin.
    if (pLoadedSkin == nullptr) {
        const QString defaultSkinName = getDefaultSkinName();
        if (defaultSkinName == pSkin->name()) {
            qCritical() << "Configured skin " << pSkin->name()
                        << " failed to load, no fallback available (it already "
                           "is the default skin)";
        } else {
            qWarning() << "Configured skin " << pSkin->name()
                       << " failed to load, falling back to default skin "
                       << defaultSkinName;
            pSkin = getSkin(defaultSkinName);
            // If we don't have a skin then fail.
            VERIFY_OR_DEBUG_ASSERT(pSkin != nullptr && pSkin->isValid()) {
                qCritical() << "Default skin" << defaultSkinName << "not found";
                return nullptr;
            }

            // This might also fail, but
            pLoadedSkin = pSkin->loadSkin(pParent, m_pConfig, pSkinCreatedControls, pCoreServices);
        }
        DEBUG_ASSERT(pLoadedSkin != nullptr);
    }

    VERIFY_OR_DEBUG_ASSERT(pLoadedSkin != nullptr) {
        qCritical() << "No skin can be loaded, please check your installation.";
    }
    return pLoadedSkin;
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

SkinPointer SkinLoader::skinFromDirectory(const QDir& dir) const {
    SkinPointer pSkin = LegacySkin::fromDirectory(dir);
    if (pSkin && pSkin->isValid()) {
        return pSkin;
    }

    if (m_pConfig->getValue(ConfigKey("[Config]", "experimental_qml_skin_support"), false)) {
        pSkin = QmlSkin::fromDirectory(dir);
        if (pSkin && pSkin->isValid()) {
            return pSkin;
        }
    }

    return nullptr;
}

} // namespace skin
} // namespace mixxx
