#include "qml/qmlconfigproxy.h"

#include <qglobal.h>

#include "library/basetracktablemodel.h"
#include "library/library.h"
#include "moc_qmlconfigproxy.cpp"
#include "preferences/colorpalettesettings.h"
#include "preferences/constants.h"
#include "util/color/predefinedcolorpalettes.h"

#define PROPERTY_IMPL_GETTER(GROUP, KEY, TYPE, NAME, DEFAULT) \
    TYPE QmlConfigProxy::NAME() const {                       \
        return m_pConfig->getValue(                           \
                ConfigKey(GROUP, KEY),                        \
                DEFAULT);                                     \
    }

#define PROPERTY_IMPL(GROUP, KEY, TYPE, NAME, DEFAULT)                      \
    PROPERTY_IMPL_GETTER(GROUP, KEY, TYPE, NAME, DEFAULT)                   \
    void QmlConfigProxy::set_##NAME(                                        \
            std::conditional<(sizeof(TYPE) <= 16), TYPE, const TYPE&>::type \
                    value) {                                                \
        if (value == DEFAULT) {                                             \
            m_pConfig->remove(ConfigKey(GROUP, KEY));                       \
            return;                                                         \
        }                                                                   \
        m_pConfig->setValue(ConfigKey(GROUP, KEY), value);                  \
        emit NAME##Changed();                                               \
    }

namespace {
QVariantList paletteToQColorList(const ColorPalette& palette) {
    QVariantList colors;
    for (mixxx::RgbColor rgbColor : palette) {
        colors.append(mixxx::RgbColor::toQVariantColor(rgbColor));
    }
    return colors;
}

const QString kPreferencesGroup = QStringLiteral("[Preferences]");
const QString kConfigGroup = QStringLiteral("[Config]");
const QString kControlGroup = QStringLiteral("[Control]");
const QString kLibraryGroup = QStringLiteral("[Library]");
const QString kBpmGroup = QStringLiteral("[BPM]");

const QString kMultiSamplingKey = QStringLiteral("multi_sampling");
const QString k3DHardwareAccelerationKey = QStringLiteral("force_hardware_acceleration");

const QString kWaveformGroup = QStringLiteral("[Waveform]");
const QString kWaveformZoomSynchronizationKey = QStringLiteral("ZoomSynchronization");
const QString kWaveformDefaultZoomKey = QStringLiteral("DefaultZoom");
const bool kWaveformZoomSynchronizationDefault = true;
const double kWaveformDefaultZoomDefault = 3.0;

// Library group
const QString kTooltipsKey = QStringLiteral("Tooltips");
const QString kInhibitScreensaverKey = QStringLiteral("InhibitScreensaver");
const QString kHideMenuBarKey = QStringLiteral("hide_menubar");
const QString kEnableSearchCompletionsKey = QStringLiteral("EnableSearchCompletions");
const QString kEnableSearchHistoryShortcutsKey = QStringLiteral("EnableSearchHistoryShortcuts");
const QString kBpmColumnPrecisionKey = QStringLiteral("BpmColumnPrecision");
const QString kRowHeightKey = QStringLiteral("RowHeight");

// Controls group
const QString kHotcueDefaultColorIndexKey = QStringLiteral("HotcueDefaultColorIndex");
const QString kLoopDefaultColorIndexKey = QStringLiteral("LoopDefaultColorIndex");
const QString kCueDefaultKey = QStringLiteral("CueDefault");
const QString kSetIntroStartAtMainCueKey = QStringLiteral("SetIntroStartAtMainCue");
const QString kCloneDeckOnLoadDoubleTapKey = QStringLiteral("CloneDeckOnLoadDoubleTap");
const QString kLoadWhenDeckPlayingKey = QStringLiteral("LoadWhenDeckPlaying");
const QString kTimeFormatKey = QStringLiteral("TimeFormat");           // TrackTime::DisplayFormat
const QString kPositionDisplayKey = QStringLiteral("PositionDisplay"); // TrackTime::DisplayMode
const QString kCueRecallKey = QStringLiteral("CueRecall");             // SeekOnLoadMode
const QString kSpeedAutoResetKey =
        QStringLiteral("SpeedAutoReset"); // BaseTrackPlayer::TrackLoadReset
const QString kKeylockModeKey = QStringLiteral("keylockMode");
const QString kKeyunlockModeKey = QStringLiteral("keyunlockMode");
const QString kRateRampSensitivityKey = QStringLiteral("RateRampSensitivity");
const QString kRateTempCoarseKey = QStringLiteral("RateTempLeft");
const QString kRateTempFineKey = QStringLiteral("RateTempRight");
const QString kRatePermCoarseKey = QStringLiteral("RatePermLeft");
const QString kRatePermFineKey = QStringLiteral("RatePermRight");
const QString kRateRangeKey = QStringLiteral("RateRangePercent");
const QString kRateDirKey = QStringLiteral("RateDir");
const QString kRateRampKey = QStringLiteral("RateRamp");

// Config group
const QString kHotcueColorPaletteKey = QStringLiteral("HotcueColorPalette");
const QString kTrackColorPaletteKey = QStringLiteral("TrackColorPalette");
const QString kKeyColorPaletteKey = QStringLiteral("KeyColorPalette");
const QString kStartInFullscreenKey = QStringLiteral("StartInFullscreen");
const QString kKeyColorsEnabledKey = QStringLiteral("key_colors_enabled");

// BPM group
const QString kSyncLockAlgorithmKey = QStringLiteral("sync_lock_algorithm");

} // namespace

namespace mixxx {
namespace qml {

QmlConfigProxy::QmlConfigProxy(
        UserSettingsPointer pConfig, QObject* parent)
        : QObject(parent), m_pConfig(pConfig) {
}

QVariantList QmlConfigProxy::hotcueColorPalette() const {
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    return paletteToQColorList(colorPaletteSettings.getHotcueColorPalette());
}

QVariantList QmlConfigProxy::getHotcueColorPalette(const QString& paletteName) const {
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    return paletteToQColorList(colorPaletteSettings.getHotcueColorPalette(paletteName));
}

void QmlConfigProxy::setHotcueColorPalette(const QString& paletteName) {
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    colorPaletteSettings.setHotcueColorPalette(colorPaletteSettings.getColorPalette(paletteName,
            colorPaletteSettings.getHotcueColorPalette()));
    emit hotcueColorPaletteChanged();
}

QVariantList QmlConfigProxy::trackColorPalette() const {
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    return paletteToQColorList(colorPaletteSettings.getTrackColorPalette());
}

QVariantList QmlConfigProxy::getTrackColorPalette(const QString& paletteName) const {
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    return paletteToQColorList(colorPaletteSettings.getTrackColorPalette(paletteName));
}
void QmlConfigProxy::setTrackColorPalette(const QString& paletteName) {
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    colorPaletteSettings.setTrackColorPalette(colorPaletteSettings.getColorPalette(paletteName,
            colorPaletteSettings.getTrackColorPalette()));
    emit trackColorPaletteChanged();
}

QVariantList QmlConfigProxy::keyColorPalette() const {
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    return paletteToQColorList(colorPaletteSettings.getConfigKeyColorPalette());
}

QVariantList QmlConfigProxy::getKeyColorPalette(const QString& paletteName) const {
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    return paletteToQColorList(colorPaletteSettings.getColorPalette(paletteName,
            mixxx::PredefinedColorPalettes::kDefaultKeyColorPalette));
}

void QmlConfigProxy::setKeyColorPalette(const QString& paletteName) {
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    colorPaletteSettings.setKeyColorPalette(colorPaletteSettings.getColorPalette(paletteName,
            colorPaletteSettings.getConfigKeyColorPalette()));
    emit keyColorPaletteChanged();
}

QVariantList QmlConfigProxy::colorPalette(const QString& paletteName) const {
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    return paletteToQColorList(colorPaletteSettings.getColorPalette(paletteName,
            mixxx::PredefinedColorPalettes::kDefaultKeyColorPalette));
}

QStringList QmlConfigProxy::paletteNames() const {
    QStringList palettes;
    for (const auto& palette : mixxx::PredefinedColorPalettes::kPalettes) {
        palettes.append(palette.getName());
    }
    return palettes;
}

PROPERTY_IMPL(kPreferencesGroup,
        kMultiSamplingKey,
        mixxx::preferences::MultiSamplingMode,
        multiSamplingLevel,
        mixxx::preferences::MultiSamplingMode::Disabled);

bool QmlConfigProxy::useAcceleration() const {
    if (!m_pConfig->exists(
                ConfigKey(kPreferencesGroup, k3DHardwareAccelerationKey))) {
        // TODO: detect whether QML currently run with 3D acceleration. QSGRendererInterface?
        return false;
    }
    return m_pConfig->getValue<bool>(
            ConfigKey(kPreferencesGroup, k3DHardwareAccelerationKey));
}

void QmlConfigProxy::set_useAcceleration(bool value) {
    m_pConfig->setValue(
            ConfigKey(kPreferencesGroup, k3DHardwareAccelerationKey), value);
    emit useAccelerationChanged();
}

bool QmlConfigProxy::waveformZoomSynchronization() {
    return m_pConfig->getValue(
            ConfigKey(kWaveformGroup, kWaveformZoomSynchronizationKey),
            kWaveformZoomSynchronizationDefault);
}
double QmlConfigProxy::waveformDefaultZoom() {
    return m_pConfig->getValue(
            ConfigKey(kWaveformGroup, kWaveformDefaultZoomKey),
            kWaveformDefaultZoomDefault);
}

PROPERTY_IMPL(kLibraryGroup,
        kTooltipsKey,
        mixxx::preferences::Tooltips,
        libraryTooltips,
        mixxx::preferences::Tooltips::On);
PROPERTY_IMPL(kLibraryGroup,
        kInhibitScreensaverKey,
        mixxx::preferences::ScreenSaver,
        libraryInhibitScreensaver,
        mixxx::preferences::ScreenSaver::On);
PROPERTY_IMPL(kLibraryGroup, kHideMenuBarKey, bool, libraryHideMenuBar, false);
PROPERTY_IMPL(kLibraryGroup,
        kEnableSearchCompletionsKey,
        bool,
        libraryEnableSearchCompletions,
        true);
PROPERTY_IMPL(kLibraryGroup,
        kEnableSearchHistoryShortcutsKey,
        bool,
        libraryEnableSearchHistoryShortcuts,
        true);
PROPERTY_IMPL(kLibraryGroup,
        kBpmColumnPrecisionKey,
        int,
        libraryBpmColumnPrecision,
        BaseTrackTableModel::kBpmColumnPrecisionDefault);
PROPERTY_IMPL(kLibraryGroup, kRowHeightKey, double, libraryRowHeight, Library::kDefaultRowHeightPx);
PROPERTY_IMPL(kControlGroup, kHotcueDefaultColorIndexKey, int, controlHotcueDefaultColorIndex, -1);
PROPERTY_IMPL(kControlGroup, kLoopDefaultColorIndexKey, double, controlLoopDefaultColorIndex, -1);
PROPERTY_IMPL(kControlGroup, kCueDefaultKey, CueMode, controlCueDefault, CueMode::Mixxx);
PROPERTY_IMPL(kControlGroup, kSetIntroStartAtMainCueKey, bool, controlSetIntroStartAtMainCue, true);
PROPERTY_IMPL(kControlGroup,
        kCloneDeckOnLoadDoubleTapKey,
        bool,
        controlCloneDeckOnLoadDoubleTap,
        true);
PROPERTY_IMPL(kControlGroup,
        kLoadWhenDeckPlayingKey,
        LoadWhenDeckPlaying,
        controlLoadWhenDeckPlaying,
        LoadWhenDeckPlaying::Reject);
PROPERTY_IMPL(kControlGroup,
        kTimeFormatKey,
        TrackTime::DisplayFormat,
        controlTimeFormat,
        TrackTime::DisplayFormat::TRADITIONAL);
PROPERTY_IMPL(kControlGroup,
        kPositionDisplayKey,
        TrackTime::DisplayMode,
        controlPositionDisplay,
        TrackTime::DisplayMode::ELAPSED_AND_REMAINING);
PROPERTY_IMPL(kControlGroup,
        kCueRecallKey,
        SeekOnLoadMode,
        controlCueRecall,
        SeekOnLoadMode::IntroStart);
PROPERTY_IMPL(kControlGroup,
        kSpeedAutoResetKey,
        BaseTrackPlayer::TrackLoadReset,
        controlSpeedAutoReset,
        BaseTrackPlayer::TrackLoadReset::RESET_PITCH);
PROPERTY_IMPL(kControlGroup,
        kKeylockModeKey,
        KeylockMode,
        controlKeylockMode,
        KeylockMode::LockOriginalKey);
PROPERTY_IMPL(kControlGroup,
        kKeyunlockModeKey,
        KeyunlockMode,
        controlKeyunlockMode,
        KeyunlockMode::ResetLockedKey);
PROPERTY_IMPL(kControlGroup, kRateRampSensitivityKey, double, controlRateRampSensitivity, 250);
PROPERTY_IMPL(kControlGroup, kRateTempCoarseKey, double, controlRateTempCoarse, 4.00);
PROPERTY_IMPL(kControlGroup, kRateTempFineKey, double, controlRateTempFine, 2.00);
PROPERTY_IMPL(kControlGroup, kRatePermCoarseKey, double, controlRatePermCoarse, 0.50);
PROPERTY_IMPL(kControlGroup, kRatePermFineKey, double, controlRatePermFine, 0.05);
PROPERTY_IMPL(kControlGroup, kRateRangeKey, int, controlRateRange, 8);
PROPERTY_IMPL(kControlGroup, kRateDirKey, bool, controlRateDir, true);
PROPERTY_IMPL(kControlGroup,
        kRateRampKey,
        RateControl::RampMode,
        controlPitchBendBehaviour,
        RateControl::RampMode::Stepping);

// Config group
PROPERTY_IMPL(kConfigGroup,
        kHotcueColorPaletteKey,
        QString,
        configHotcueColorPalette,
        mixxx::PredefinedColorPalettes::kDefaultHotcueColorPalette.getName());
PROPERTY_IMPL(kConfigGroup,
        kTrackColorPaletteKey,
        QString,
        configTrackColorPalette,
        PredefinedColorPalettes::kDefaultTrackColorPalette.getName());
PROPERTY_IMPL(kConfigGroup,
        kKeyColorPaletteKey,
        QString,
        configKeyColorPalette,
        PredefinedColorPalettes::kDefaultKeyColorPalette.getName());
PROPERTY_IMPL(kControlGroup,
        kKeyColorsEnabledKey,
        bool,
        configKeyColorsEnabled,
        BaseTrackTableModel::kKeyColorsEnabledDefault);
PROPERTY_IMPL(kConfigGroup,
        kStartInFullscreenKey,
        bool,
        configStartInFullscreenKey,
        false);
PROPERTY_IMPL(kBpmGroup,
        kSyncLockAlgorithmKey,
        EngineSync::SyncLockAlgorithm,
        bpmSyncLockAlgorithm,
        EngineSync::SyncLockAlgorithm::PREFER_SOFT_LEADER);

// static
QmlConfigProxy* QmlConfigProxy::create(QQmlEngine* pQmlEngine, QJSEngine*) {
    // The implementation of this method is mostly taken from the code example
    // that shows the replacement for `qmlRegisterSingletonInstance()` when
    // using `QML_SINGLETON`.
    // https://doc.qt.io/qt-6/qqmlengine.html#QML_SINGLETON

    // The instance has to exist before it is used. We cannot replace it.
    VERIFY_OR_DEBUG_ASSERT(s_pUserSettings) {
        qWarning() << "UserSettings hasn't been registered yet";
        return nullptr;
    }
    return new QmlConfigProxy(s_pUserSettings, pQmlEngine);
}

} // namespace qml
} // namespace mixxx
