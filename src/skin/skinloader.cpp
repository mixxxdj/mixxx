#include "skin/skinloader.h"

#include <QDir>
#include <QString>
#include <QtDebug>

#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "mixer/playermanager.h"
#include "moc_skinloader.cpp"
#include "skin/legacy/launchimage.h"
#include "skin/legacy/legacyskin.h"
#include "skin/legacy/legacyskinparser.h"
#include "util/debug.h"
#include "util/timer.h"

const QString kSkinsDirName = QStringLiteral("skins");

namespace mixxx {
namespace skin {

using legacy::LegacySkin;

SkinLoader::SkinLoader(UserSettingsPointer pConfig)
        : m_pConfig(pConfig),
          m_spinnyCoverControlsCreated(false),
          m_micDuckingControlsCreated(false),
          m_numMicsEnabled(1) {
}

SkinLoader::~SkinLoader() {
    LegacySkinParser::clearSharedGroupStrings();
}

QList<SkinPointer> SkinLoader::getUserSkins() const {
    return getSkinsFromDir(getUserSkinDir());
}

QList<SkinPointer> SkinLoader::getSystemSkins() const {
    return getSkinsFromDir(getSytemSkinDir());
}

QList<SkinPointer> SkinLoader::getSkinsFromDir(const QDir& dir) const {
    QList<SkinPointer> skins;
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

    return skins;
}

QList<QDir> SkinLoader::getSkinSearchPaths() const {
    QList<QDir> searchPaths;

    const auto userSkinDir = getUserSkinDir();
    if (!userSkinDir.path().isEmpty()) {
        searchPaths.append(userSkinDir);
    }

    searchPaths.append(getSytemSkinDir());

    return searchPaths;
}

QDir SkinLoader::getUserSkinDir() const {
    QDir userSkinsPath(m_pConfig->getSettingsPath());
    if (userSkinsPath.cd(kSkinsDirName)) {
        return userSkinsPath;
    }
    return {};
}

QDir SkinLoader::getSytemSkinDir() const {
    // If we can't find the skins folder then we can't load a skin at all. This
    // is a critical error in the user's Mixxx installation.
    QDir skinsPath(m_pConfig->getResourcePath());
    if (!skinsPath.cd(kSkinsDirName)) {
        reportCriticalErrorAndQuit("Skin directory does not exist: " +
                skinsPath.absoluteFilePath(kSkinsDirName));
    }
    return skinsPath;
}

SkinPointer SkinLoader::getSkin(const QString& skinName) const {
    // If there are skins with identical name in both the resource and user
    // directory, we'll discover the one from the user dir first
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
        return pSkin;
    }
    qWarning() << "Failed to find configured skin" << configSkin;

    // Fallback to default skin as last resort
    const QString defaultSkinName = getDefaultSkinName();
    DEBUG_ASSERT(!defaultSkinName.isEmpty());
    pSkin = getSkin(defaultSkinName);
    VERIFY_OR_DEBUG_ASSERT(pSkin && pSkin->isValid()) {
        qWarning() << "Can't find default skin" << defaultSkinName;
        return nullptr;
    }
    qInfo() << "Found default skin" << defaultSkinName;
    return pSkin;
}

QString SkinLoader::getDefaultSkinName() const {
    return "LateNight";
}

QWidget* SkinLoader::loadConfiguredSkin(QWidget* pParent,
        QSet<ControlObject*>* pSkinCreatedControls,
        mixxx::CoreServices* pCoreServices) {
    ScopedTimer timer(QStringLiteral("SkinLoader::loadConfiguredSkin"));
    SkinPointer pSkin = getConfiguredSkin();

    // If we don't have a skin then fail. This makes sense here, because the
    // method above already tried to fall back to the default skin if the
    // configured one is not available. If `pSkin` is nullptr, we both the
    // configured and the default skin were not found, so there is nothing we
    // can do.
    VERIFY_OR_DEBUG_ASSERT(pSkin != nullptr && pSkin->isValid()) {
        return nullptr;
    }

    // This hooks up to and also creates some common GUI controls and some 'meta'
    // controls that allow to keep LateNight's xml structure (for cover/spinnies
    // and for the ducking GUI) simple.
    setupSpinnyCoverControls();
    // PlayerManager created all devices, but SoundManager will setup devices after
    // the skin was loaded.
    setupMicDuckingControls();

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
        DEBUG_ASSERT(pLoadedSkin);
    }

    VERIFY_OR_DEBUG_ASSERT(pLoadedSkin != nullptr) {
        qCritical() << "No skin can be loaded, please check your installation.";
    }
    qInfo() << "Loaded skin" << pSkin->name() << "from" << pSkin->path().filePath();
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

    return nullptr;
}

void SkinLoader::setupSpinnyCoverControls() {
    if (m_spinnyCoverControlsCreated) {
        return;
    }
    // Spinnies and deck cover art toggles
    m_pShowSpinny = make_parented<ControlProxy>("[Skin]", "show_spinnies", this);
    m_pShowCover = make_parented<ControlProxy>("[Skin]", "show_coverart", this);
    m_pSelectBigSpinnyCover = std::make_unique<ControlPushButton>(
            ConfigKey("[Skin]", "select_big_spinny_or_cover"), true);
    m_pSelectBigSpinnyCover->setButtonMode(mixxx::control::ButtonMode::Toggle);

    // This is 1 if [Skin], show_spinnies == 1 OR [Skin],show_coverart == 1
    m_pShowSpinnyAndOrCover = std::make_unique<ControlPushButton>(
            ConfigKey("[Skin]", "show_spinny_or_cover"));
    m_pShowSpinnyAndOrCover->setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_pShowSpinnyAndOrCover->setReadOnly();
    // This is 1 if [Skin],show_spinny_cover == 1 AND [Skin],select_big_spinny_coverart == 0
    m_pShowSmallSpinnyCover = std::make_unique<ControlPushButton>(
            ConfigKey("[Skin]", "show_small_spinny_or_cover"));
    m_pShowSmallSpinnyCover->setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_pShowSmallSpinnyCover->setReadOnly();
    // This is 1 if [Skin],show_spinny_cover == 1 AND [Skin],select_big_spinny_coverart == 1
    m_pShowBigSpinnyCover = std::make_unique<ControlPushButton>(
            ConfigKey("[Skin]", "show_big_spinny_or_cover"));
    m_pShowBigSpinnyCover->setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_pShowBigSpinnyCover->setReadOnly();

    m_pShowSpinny->connectValueChanged(this, &SkinLoader::updateSpinnyCoverControls);
    m_pShowCover->connectValueChanged(this, &SkinLoader::updateSpinnyCoverControls);
    connect(m_pSelectBigSpinnyCover.get(),
            &ControlObject::valueChanged,
            this,
            &SkinLoader::updateSpinnyCoverControls);

    m_spinnyCoverControlsCreated = true;
    updateSpinnyCoverControls();
}

void SkinLoader::updateSpinnyCoverControls() {
    if (!m_spinnyCoverControlsCreated) {
        return;
    }
    m_pShowSpinnyAndOrCover->setAndConfirm(
            (m_pShowSpinny->toBool() || m_pShowCover->toBool())
                    ? 1.0
                    : 0.0);
    m_pShowSmallSpinnyCover->setAndConfirm(
            (m_pShowSpinnyAndOrCover->toBool() && !m_pSelectBigSpinnyCover->toBool())
                    ? 1.0
                    : 0.0);
    m_pShowBigSpinnyCover->setAndConfirm(
            (m_pShowSpinnyAndOrCover->toBool() &&
                    m_pSelectBigSpinnyCover->toBool())
                    ? 1.0
                    : 0.0);
}

void SkinLoader::setupMicDuckingControls() {
    if (m_micDuckingControlsCreated) {
        return;
    }
    // This is 1 if at least one microphone device is configured
    m_pShowDuckingControls = std::make_unique<ControlPushButton>(
            ConfigKey("[Skin]", "show_ducking_controls"));
    m_pShowDuckingControls->setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_pShowDuckingControls->setReadOnly();

    m_pNumMics = make_parented<ControlProxy>(
            QStringLiteral("[App]"), QStringLiteral("num_microphones"), this);
    m_pNumMics->connectValueChanged(this, &SkinLoader::slotNumMicsChanged);

    m_micDuckingControlsCreated = true;
    slotNumMicsChanged(m_pNumMics->get());
}

void SkinLoader::slotNumMicsChanged(double dNumMics) {
    int numMics = static_cast<int>(dNumMics);

    if (numMics <= m_numMicsEnabled) {
        return;
    }

    for (int micNum = m_numMicsEnabled; micNum <= numMics; ++micNum) {
        QString micGroup = PlayerManager::groupForMicrophone(micNum - 1);
        ControlProxy* pMicEnabled = new ControlProxy(micGroup, "input_configured", this);
        m_pMicConfiguredControls.push_back(pMicEnabled);
        pMicEnabled->connectValueChanged(this, &SkinLoader::updateDuckingControl);
    }
    m_numMicsEnabled = numMics;

    updateDuckingControl();
}

void SkinLoader::updateDuckingControl() {
    if (!m_micDuckingControlsCreated) {
        return;
    }
    double atLeastOneMicConfigured = 0.0;
    for (auto* pMicCon : std::as_const(m_pMicConfiguredControls)) {
        if (pMicCon->toBool()) {
            atLeastOneMicConfigured = 1.0;
            break;
        }
    }
    m_pShowDuckingControls->setAndConfirm(atLeastOneMicConfigured);
}
} // namespace skin
} // namespace mixxx
