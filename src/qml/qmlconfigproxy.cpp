#include "qml/qmlconfigproxy.h"

#include <Qt>
#include <algorithm>

#include "control/controlobject.h"
#include "library/basetracktablemodel.h"
#include "library/library.h"
#include "library/library_prefs.h"
#include "moc_qmlconfigproxy.cpp"
#include "preferences/colorpalettesettings.h"
#include "preferences/constants.h"
#include "util/color/predefinedcolorpalettes.h"
#include "waveform/waveformwidgetfactory.h"

#define PROPERTY_IMPL_GETTER(GROUP, KEY, TYPE, NAME, DEFAULT) \
    TYPE QmlConfigProxy::NAME() const {                       \
        return m_pConfig->getValue(                           \
                ConfigKey(GROUP, KEY),                        \
                DEFAULT);                                     \
    }

#define PROPERTY_IMPL_CONFIGKEY(CONFIGKEY, TYPE, NAME, DEFAULT) \
    PROPERTY_IMPL(CONFIGKEY.group, CONFIGKEY.item, TYPE, NAME, DEFAULT)

#define PROPERTY_IMPL(GROUP, KEY, TYPE, NAME, DEFAULT)                       \
    PROPERTY_IMPL_GETTER(GROUP, KEY, TYPE, NAME, DEFAULT)                    \
    void QmlConfigProxy::set_##NAME(                                         \
            std::conditional_t<(sizeof(TYPE) <= 16), TYPE, const TYPE&>      \
                    value) {                                                 \
        setConfigValueAndNotify<TYPE>(                                       \
                GROUP, KEY, value, DEFAULT, &QmlConfigProxy::NAME##Changed); \
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
const QString kWaveformGroup = QStringLiteral("[Waveform]");
const QString kControlsGroup = QStringLiteral("[Controls]");
const QString kLibraryGroup = QStringLiteral("[Library]");
const QString kBpmGroup = QStringLiteral("[BPM]");

const QString kMultiSamplingKey = QStringLiteral("multi_sampling");
const QString k3DHardwareAccelerationKey = QStringLiteral("force_hardware_acceleration");

// Library group
const QString kRhythmboxEnabled = QStringLiteral("ShowRhythmboxLibrary");
const QString kBansheeEnabled = QStringLiteral("ShowBansheeLibrary");
const QString kITunesEnabled = QStringLiteral("ShowITunesLibrary");
const QString kTraktorEnabled = QStringLiteral("ShowTraktorLibrary");
const QString kRekordboxEnabled = QStringLiteral("ShowRekordboxLibrary");
const QString kSeratoEnabled = QStringLiteral("ShowSeratoLibrary");

// Waveform group
const QString kZoomSynchronizationKey = QStringLiteral("ZoomSynchronization");
const QString kOverviewNormalizedKey = QStringLiteral("OverviewNormalized");
const QString kOverviewTypeKey = QStringLiteral("WaveformOverviewType");
const QString kOverviewStereoKey = QStringLiteral("overview_stereo_mode");
const QString kOverviewMinuteMarkersKey =
        QStringLiteral("draw_overview_minute_markers");
const QString kDefaultZoomKey = QStringLiteral("DefaultZoom");
const QString kFrameRateKey = QStringLiteral("FrameRate");
const QString kPlayMarkerPositionKey = QStringLiteral("PlayMarkerPosition");
const QString kUntilMarkShowBeatsKey = QStringLiteral("UntilMarkShowBeats");
const QString kUntilMarkShowTimeKey = QStringLiteral("UntilMarkShowTime");
const QString kUntilMarkAlignKey = QStringLiteral("UntilMarkAlign");
const QString kUntilMarkTextPointSizeKey = QStringLiteral("UntilMarkTextPointSize");
const QString kUntilMarkTextHeightLimitKey = QStringLiteral("UntilMarkTextHeightLimit");
const QString kVisualGainAllKey = QStringLiteral("VisualGain_0");
const QString kVisualGainLowKey = QStringLiteral("VisualGain_1");
const QString kVisualGainMediumKey = QStringLiteral("VisualGain_2");
const QString kVisualGainHighKey = QStringLiteral("VisualGain_3");
const QString kBeatGridAlphaKey = QStringLiteral("beatGridAlpha");
const QString kStemOpacityKey = QStringLiteral("stem_opacity");
const QString kStemOutlineOpacityKey = QStringLiteral("stem_outline_opacity");
const QString kStemReorderOnChangeKey = QStringLiteral("stem_reorder_on_change");
const QString kStemSplitTracksKey = QStringLiteral("stem_split_tracks");
const QString kEndOfTrackWarningTimeKey = QStringLiteral("EndOfTrackWarningTime");
const QString kWaveformTypeKey = QStringLiteral("WaveformType");
const QString kWaveformOptionsKey = QStringLiteral("waveform_options");
const QString kWaveformCachingEnabledKey = QStringLiteral("EnableWaveformCaching");
const QString kWaveformGenerationWithAnalysisEnabledKey =
        QStringLiteral("EnableWaveformGenerationWithAnalysis");

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
const QString kSchemeKey = QStringLiteral("Scheme");
const QString kResizableSkinKey = QStringLiteral("ResizableSkin");

// BPM group
const QString kSyncLockAlgorithmKey = QStringLiteral("sync_lock_algorithm");

} // namespace

namespace mixxx {
namespace qml {

QmlConfigProxy::QmlConfigProxy(
        UserSettingsPointer pConfig, QObject* pParent)
        : QmlConfigProxyBase(pParent),
          m_pConfig(pConfig) {
    QmlConfigProxyBase::s_pInstance = this;

    const ConfigKey overviewStereoKey(kWaveformGroup, kOverviewStereoKey);
    if (!ControlObject::exists(overviewStereoKey)) {
        m_pOverviewStereoControl = std::make_unique<ControlObject>(overviewStereoKey);
        m_pOverviewStereoControl->setReadOnly();
        m_pOverviewStereoControl->forceSet(waveformOverviewStereo());
    }
    const ConfigKey overviewMinuteMarkersKey(kWaveformGroup, kOverviewMinuteMarkersKey);
    if (!ControlObject::exists(overviewMinuteMarkersKey)) {
        m_pOverviewMinuteMarkersControl = std::make_unique<ControlObject>(
                overviewMinuteMarkersKey);
        m_pOverviewMinuteMarkersControl->setReadOnly();
        m_pOverviewMinuteMarkersControl->forceSet(waveformOverviewMinuteMarkers());
    }
}

QmlConfigProxy::~QmlConfigProxy() {
    if (QmlConfigProxyBase::s_pInstance == this) {
        QmlConfigProxyBase::s_pInstance = nullptr;
    }
}

void QmlConfigProxy::notifyWaveformSettingsChanged() {
    auto* pConfig = qobject_cast<QmlConfigProxy*>(QmlConfigProxyBase::s_pInstance);
    if (!pConfig) {
        return;
    }
    emit pConfig->waveformZoomSynchronizationChanged();
    emit pConfig->waveformOverviewNormalizedChanged();
    emit pConfig->waveformOverviewTypeChanged();
    emit pConfig->waveformOverviewStereoChanged();
    emit pConfig->waveformOverviewMinuteMarkersChanged();
    emit pConfig->waveformDefaultZoomChanged();
    emit pConfig->waveformPlayMarkerPositionChanged();
    emit pConfig->waveformEnabledChanged();
    emit pConfig->waveformFrameRateChanged();
    emit pConfig->waveformUntilMarkShowBeatsChanged();
    emit pConfig->waveformUntilMarkShowTimeChanged();
    emit pConfig->waveformUntilMarkAlignChanged();
    emit pConfig->waveformUntilMarkTextPointSizeChanged();
    emit pConfig->waveformUntilMarkTextHeightLimitChanged();
    emit pConfig->waveformVisualGainAllChanged();
    emit pConfig->waveformVisualGainLowChanged();
    emit pConfig->waveformVisualGainMediumChanged();
    emit pConfig->waveformVisualGainHighChanged();
    emit pConfig->waveformEndOfTrackWarningTimeChanged();
    emit pConfig->waveformTypeChanged();
    emit pConfig->waveformOptionsChanged();
    emit pConfig->waveformBeatGridAlphaChanged();
    emit pConfig->waveformStemOpacityChanged();
    emit pConfig->waveformStemOutlineOpacityChanged();
    emit pConfig->waveformStemReorderOnChangeChanged();
    emit pConfig->waveformStemSplitTracksChanged();
    emit pConfig->waveformCachingEnabledChanged();
    emit pConfig->waveformGenerationWithAnalysisEnabledChanged();
}

void QmlConfigProxy::notifyWaveformAverageFrameRateChanged() {
    auto* pConfig = qobject_cast<QmlConfigProxy*>(QmlConfigProxyBase::s_pInstance);
    if (!pConfig) {
        return;
    }
    emit pConfig->waveformAverageFrameRateChanged();
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

PROPERTY_IMPL_GETTER(
        kWaveformGroup, kZoomSynchronizationKey, bool, waveformZoomSynchronization, true);
void QmlConfigProxy::set_waveformZoomSynchronization(bool value) {
    if (WaveformWidgetFactory::isCreated()) {
        WaveformWidgetFactory::instance()->setZoomSync(value);
        emit waveformZoomSynchronizationChanged();
        return;
    }
    setConfigValueAndNotify<bool>(kWaveformGroup,
            kZoomSynchronizationKey,
            value,
            true,
            &QmlConfigProxy::waveformZoomSynchronizationChanged);
}
PROPERTY_IMPL_GETTER(kWaveformGroup,
        kOverviewNormalizedKey,
        bool,
        waveformOverviewNormalized,
        false);

void QmlConfigProxy::set_waveformOverviewNormalized(bool value) {
    setConfigValueAndNotify<bool>(kWaveformGroup,
            kOverviewNormalizedKey,
            value,
            false,
            &QmlConfigProxy::waveformOverviewNormalizedChanged);
    if (WaveformWidgetFactory::isCreated()) {
        WaveformWidgetFactory::instance()->setOverviewNormalized(value);
    }
}

PROPERTY_IMPL_GETTER(kWaveformGroup, kOverviewTypeKey, int, waveformOverviewType, 2);

void QmlConfigProxy::set_waveformOverviewType(int value) {
    const int sanitizedValue = std::clamp(value, 0, 2);
    setConfigValueAndNotify<int>(kWaveformGroup,
            kOverviewTypeKey,
            sanitizedValue,
            2,
            &QmlConfigProxy::waveformOverviewTypeChanged);
    if (auto* pControl = ControlObject::getControl(
                ConfigKey(kWaveformGroup, kOverviewTypeKey))) {
        pControl->forceSet(sanitizedValue);
    }
}

PROPERTY_IMPL_GETTER(kWaveformGroup,
        kOverviewStereoKey,
        bool,
        waveformOverviewStereo,
        true);

void QmlConfigProxy::set_waveformOverviewStereo(bool value) {
    setConfigValueAndNotify<bool>(kWaveformGroup,
            kOverviewStereoKey,
            value,
            true,
            &QmlConfigProxy::waveformOverviewStereoChanged);
    if (auto* pControl = ControlObject::getControl(
                ConfigKey(kWaveformGroup, kOverviewStereoKey))) {
        pControl->forceSet(value);
    }
}

PROPERTY_IMPL_GETTER(kWaveformGroup,
        kOverviewMinuteMarkersKey,
        bool,
        waveformOverviewMinuteMarkers,
        true);

void QmlConfigProxy::set_waveformOverviewMinuteMarkers(bool value) {
    setConfigValueAndNotify<bool>(kWaveformGroup,
            kOverviewMinuteMarkersKey,
            value,
            true,
            &QmlConfigProxy::waveformOverviewMinuteMarkersChanged);
    if (auto* pControl = ControlObject::getControl(
                ConfigKey(kWaveformGroup, kOverviewMinuteMarkersKey))) {
        pControl->forceSet(value);
    }
}

PROPERTY_IMPL_GETTER(kWaveformGroup, kDefaultZoomKey, double, waveformDefaultZoom, 3);
void QmlConfigProxy::set_waveformDefaultZoom(double value) {
    if (WaveformWidgetFactory::isCreated()) {
        WaveformWidgetFactory::instance()->setDefaultZoom(value);
        emit waveformDefaultZoomChanged();
        return;
    }
    setConfigValueAndNotify<double>(kWaveformGroup,
            kDefaultZoomKey,
            value,
            3,
            &QmlConfigProxy::waveformDefaultZoomChanged);
}
PROPERTY_IMPL_GETTER(kWaveformGroup, kFrameRateKey, int, waveformFrameRate, 60);
void QmlConfigProxy::set_waveformFrameRate(int frameRate) {
    const int sanitizedFrameRate = std::clamp(frameRate, 1, 240);
    if (WaveformWidgetFactory::isCreated()) {
        WaveformWidgetFactory::instance()->setFrameRate(sanitizedFrameRate);
        emit waveformFrameRateChanged();
        return;
    }
    setConfigValueAndNotify<int>(kWaveformGroup,
            kFrameRateKey,
            sanitizedFrameRate,
            60,
            &QmlConfigProxy::waveformFrameRateChanged);
}
PROPERTY_IMPL_GETTER(kWaveformGroup,
        kPlayMarkerPositionKey,
        double,
        waveformPlayMarkerPosition,
        0.5);
void QmlConfigProxy::set_waveformPlayMarkerPosition(double value) {
    const double sanitizedValue = std::clamp(value, 0.0, 1.0);
    if (WaveformWidgetFactory::isCreated()) {
        WaveformWidgetFactory::instance()->setPlayMarkerPosition(sanitizedValue);
        emit waveformPlayMarkerPositionChanged();
        return;
    }
    setConfigValueAndNotify<double>(kWaveformGroup,
            kPlayMarkerPositionKey,
            sanitizedValue,
            0.5,
            &QmlConfigProxy::waveformPlayMarkerPositionChanged);
}

PROPERTY_IMPL_GETTER(kWaveformGroup,
        kUntilMarkShowBeatsKey,
        bool,
        waveformUntilMarkShowBeats,
        false);
void QmlConfigProxy::set_waveformUntilMarkShowBeats(bool value) {
    if (WaveformWidgetFactory::isCreated()) {
        WaveformWidgetFactory::instance()->setUntilMarkShowBeats(value);
        emit waveformUntilMarkShowBeatsChanged();
        return;
    }
    setConfigValueAndNotify<bool>(kWaveformGroup,
            kUntilMarkShowBeatsKey,
            value,
            false,
            &QmlConfigProxy::waveformUntilMarkShowBeatsChanged);
}

PROPERTY_IMPL_GETTER(kWaveformGroup, kUntilMarkShowTimeKey, bool, waveformUntilMarkShowTime, false);
void QmlConfigProxy::set_waveformUntilMarkShowTime(bool value) {
    if (WaveformWidgetFactory::isCreated()) {
        WaveformWidgetFactory::instance()->setUntilMarkShowTime(value);
        emit waveformUntilMarkShowTimeChanged();
        return;
    }
    setConfigValueAndNotify<bool>(kWaveformGroup,
            kUntilMarkShowTimeKey,
            value,
            false,
            &QmlConfigProxy::waveformUntilMarkShowTimeChanged);
}

PROPERTY_IMPL_GETTER(kWaveformGroup, kUntilMarkAlignKey, double, waveformUntilMarkAlign, 1);
void QmlConfigProxy::set_waveformUntilMarkAlign(double value) {
    const int sanitizedIndex = std::clamp(static_cast<int>(value), 0, 2);
    if (WaveformWidgetFactory::isCreated()) {
        WaveformWidgetFactory::instance()->setUntilMarkAlign(
                WaveformWidgetFactory::toUntilMarkAlign(sanitizedIndex));
        emit waveformUntilMarkAlignChanged();
        return;
    }
    setConfigValueAndNotify<int>(kWaveformGroup,
            kUntilMarkAlignKey,
            sanitizedIndex,
            1,
            &QmlConfigProxy::waveformUntilMarkAlignChanged);
}

PROPERTY_IMPL_GETTER(
        kWaveformGroup, kUntilMarkTextPointSizeKey, int, waveformUntilMarkTextPointSize, 24);
void QmlConfigProxy::set_waveformUntilMarkTextPointSize(int value) {
    if (WaveformWidgetFactory::isCreated()) {
        WaveformWidgetFactory::instance()->setUntilMarkTextPointSize(value);
        emit waveformUntilMarkTextPointSizeChanged();
        return;
    }
    setConfigValueAndNotify<int>(kWaveformGroup,
            kUntilMarkTextPointSizeKey,
            value,
            24,
            &QmlConfigProxy::waveformUntilMarkTextPointSizeChanged);
}

double QmlConfigProxy::waveformUntilMarkTextHeightLimit() const {
    if (WaveformWidgetFactory::isCreated()) {
        return WaveformWidgetFactory::instance()->getUntilMarkTextHeightLimit();
    }
    const int index = std::clamp(
            m_pConfig->getValue<int>(ConfigKey(kWaveformGroup, kUntilMarkTextHeightLimitKey), 0),
            0,
            1);
    return WaveformWidgetFactory::toUntilMarkTextHeightLimit(index);
}

void QmlConfigProxy::set_waveformUntilMarkTextHeightLimit(double value) {
    // The legacy preference has two discrete values (one third or the full
    // waveform height). Keep the QML-facing property in the same units while
    // accepting the floating-point value emitted by QML bindings.
    const float sanitizedValue = value >= 0.666 ? 1.0f : 0.333f;
    if (WaveformWidgetFactory::isCreated()) {
        WaveformWidgetFactory::instance()->setUntilMarkTextHeightLimit(sanitizedValue);
        emit waveformUntilMarkTextHeightLimitChanged();
        return;
    }

    if (sanitizedValue == 0.333f) {
        m_pConfig->remove(ConfigKey(kWaveformGroup, kUntilMarkTextHeightLimitKey));
    } else {
        m_pConfig->setValue(ConfigKey(kWaveformGroup, kUntilMarkTextHeightLimitKey), 1);
    }
    emit waveformUntilMarkTextHeightLimitChanged();
}
double QmlConfigProxy::waveformVisualGainAll() const {
    if (WaveformWidgetFactory::isCreated()) {
        return WaveformWidgetFactory::instance()->getVisualGain(BandIndex::AllBand);
    }
    return m_pConfig->getValue(ConfigKey(kWaveformGroup, kVisualGainAllKey), 2.0);
}
void QmlConfigProxy::set_waveformVisualGainAll(double value) {
    if (WaveformWidgetFactory::isCreated()) {
        WaveformWidgetFactory::instance()->setVisualGain(BandIndex::AllBand, value);
        emit waveformVisualGainAllChanged();
        return;
    }
    setConfigValueAndNotify<double>(kWaveformGroup,
            kVisualGainAllKey,
            value,
            2,
            &QmlConfigProxy::waveformVisualGainAllChanged);
}

double QmlConfigProxy::waveformVisualGainLow() const {
    if (WaveformWidgetFactory::isCreated()) {
        return WaveformWidgetFactory::instance()->getVisualGain(BandIndex::Low);
    }
    return m_pConfig->getValue(ConfigKey(kWaveformGroup, kVisualGainLowKey), 1.0);
}
void QmlConfigProxy::set_waveformVisualGainLow(double value) {
    if (WaveformWidgetFactory::isCreated()) {
        WaveformWidgetFactory::instance()->setVisualGain(BandIndex::Low, value);
        emit waveformVisualGainLowChanged();
        return;
    }
    setConfigValueAndNotify<double>(kWaveformGroup,
            kVisualGainLowKey,
            value,
            1,
            &QmlConfigProxy::waveformVisualGainLowChanged);
}

double QmlConfigProxy::waveformVisualGainMedium() const {
    if (WaveformWidgetFactory::isCreated()) {
        return WaveformWidgetFactory::instance()->getVisualGain(BandIndex::Mid);
    }
    return m_pConfig->getValue(ConfigKey(kWaveformGroup, kVisualGainMediumKey), 1.0);
}
void QmlConfigProxy::set_waveformVisualGainMedium(double value) {
    if (WaveformWidgetFactory::isCreated()) {
        WaveformWidgetFactory::instance()->setVisualGain(BandIndex::Mid, value);
        emit waveformVisualGainMediumChanged();
        return;
    }
    setConfigValueAndNotify<double>(kWaveformGroup,
            kVisualGainMediumKey,
            value,
            1,
            &QmlConfigProxy::waveformVisualGainMediumChanged);
}

double QmlConfigProxy::waveformVisualGainHigh() const {
    if (WaveformWidgetFactory::isCreated()) {
        return WaveformWidgetFactory::instance()->getVisualGain(BandIndex::High);
    }
    return m_pConfig->getValue(ConfigKey(kWaveformGroup, kVisualGainHighKey), 1.0);
}
void QmlConfigProxy::set_waveformVisualGainHigh(double value) {
    if (WaveformWidgetFactory::isCreated()) {
        WaveformWidgetFactory::instance()->setVisualGain(BandIndex::High, value);
        emit waveformVisualGainHighChanged();
        return;
    }
    setConfigValueAndNotify<double>(kWaveformGroup,
            kVisualGainHighKey,
            value,
            1,
            &QmlConfigProxy::waveformVisualGainHighChanged);
}

PROPERTY_IMPL_GETTER(
        kWaveformGroup, kEndOfTrackWarningTimeKey, int, waveformEndOfTrackWarningTime, 30);
void QmlConfigProxy::set_waveformEndOfTrackWarningTime(int value) {
    const int sanitizedValue = std::clamp(value, 0, 120);
    if (WaveformWidgetFactory::isCreated()) {
        WaveformWidgetFactory::instance()->setEndOfTrackWarningTime(sanitizedValue);
        emit waveformEndOfTrackWarningTimeChanged();
        return;
    }
    setConfigValueAndNotify<int>(kWaveformGroup,
            kEndOfTrackWarningTimeKey,
            sanitizedValue,
            30,
            &QmlConfigProxy::waveformEndOfTrackWarningTimeChanged);
}
PROPERTY_IMPL_GETTER(kWaveformGroup,
        kWaveformTypeKey,
        QmlWaveformDisplay::Type,
        waveformType,
        QmlWaveformDisplay::Type::RGB);
void QmlConfigProxy::set_waveformType(QmlWaveformDisplay::Type type) {
    if (WaveformWidgetFactory::isCreated()) {
        WaveformWidgetFactory::instance()->setWidgetType(
                static_cast<WaveformWidgetType::Type>(type));
        emit waveformTypeChanged();
    } else {
        setConfigValueAndNotify<QmlWaveformDisplay::Type>(kWaveformGroup,
                kWaveformTypeKey,
                type,
                QmlWaveformDisplay::Type::RGB,
                &QmlConfigProxy::waveformTypeChanged);
    }
    emit waveformEnabledChanged();
}

bool QmlConfigProxy::waveformEnabled() const {
    return m_pConfig->getValue<int>(
                   ConfigKey(kWaveformGroup, kWaveformTypeKey),
                   static_cast<int>(QmlWaveformDisplay::Type::RGB)) !=
            static_cast<int>(WaveformWidgetType::Empty);
}

void QmlConfigProxy::set_waveformEnabled(bool enabled) {
    if (enabled == waveformEnabled()) {
        return;
    }
    if (enabled) {
        set_waveformType(QmlWaveformDisplay::Type::RGB);
    } else {
        if (WaveformWidgetFactory::isCreated()) {
            WaveformWidgetFactory::instance()->setWidgetType(WaveformWidgetType::Empty);
        } else {
            m_pConfig->setValue(
                    ConfigKey(kWaveformGroup, kWaveformTypeKey),
                    static_cast<int>(WaveformWidgetType::Empty));
        }
        emit waveformTypeChanged();
        emit waveformEnabledChanged();
    }
}
QmlWaveformDisplay::Options QmlConfigProxy::waveformOptions() const {
    return QmlWaveformDisplay::Options::fromInt(m_pConfig->getValue<int>(
            ConfigKey(kWaveformGroup,
                    kWaveformOptionsKey),
            static_cast<int>(QmlWaveformDisplay::Option::None)));
}

void QmlConfigProxy::set_waveformOptions(QmlWaveformDisplay::Options value) {
    if (value == QmlWaveformDisplay::Option::None) {
        m_pConfig->remove(ConfigKey(kWaveformGroup, kWaveformOptionsKey));
    } else {
        m_pConfig->setValue(ConfigKey(kWaveformGroup, kWaveformOptionsKey),
                static_cast<int>(value));
    }
    emit waveformOptionsChanged();
}

PROPERTY_IMPL_GETTER(kWaveformGroup, kBeatGridAlphaKey, double, waveformBeatGridAlpha, 90);
void QmlConfigProxy::set_waveformBeatGridAlpha(double value) {
    const double sanitizedValue = std::clamp(value, 0.0, 100.0);
    if (WaveformWidgetFactory::isCreated()) {
        WaveformWidgetFactory::instance()->setDisplayBeatGridAlpha(
                static_cast<int>(sanitizedValue));
        emit waveformBeatGridAlphaChanged();
        return;
    }
    setConfigValueAndNotify<double>(kWaveformGroup,
            kBeatGridAlphaKey,
            sanitizedValue,
            90,
            &QmlConfigProxy::waveformBeatGridAlphaChanged);
}

PROPERTY_IMPL_GETTER(kWaveformGroup, kStemOpacityKey, double, waveformStemOpacity, 0.75);
void QmlConfigProxy::set_waveformStemOpacity(double value) {
    const double sanitizedValue = std::clamp(value, 0.0, 1.0);
    if (WaveformWidgetFactory::isCreated()) {
        WaveformWidgetFactory::instance()->setStemOpacity(static_cast<float>(sanitizedValue));
        emit waveformStemOpacityChanged();
        return;
    }
    setConfigValueAndNotify<double>(kWaveformGroup,
            kStemOpacityKey,
            sanitizedValue,
            0.75,
            &QmlConfigProxy::waveformStemOpacityChanged);
}

PROPERTY_IMPL_GETTER(
        kWaveformGroup, kStemOutlineOpacityKey, double, waveformStemOutlineOpacity, 0.15);
void QmlConfigProxy::set_waveformStemOutlineOpacity(double value) {
    const double sanitizedValue = std::clamp(value, 0.0, 1.0);
    if (WaveformWidgetFactory::isCreated()) {
        WaveformWidgetFactory::instance()->setStemOutlineOpacity(
                static_cast<float>(sanitizedValue));
        emit waveformStemOutlineOpacityChanged();
        return;
    }
    setConfigValueAndNotify<double>(kWaveformGroup,
            kStemOutlineOpacityKey,
            sanitizedValue,
            0.15,
            &QmlConfigProxy::waveformStemOutlineOpacityChanged);
}

PROPERTY_IMPL_GETTER(
        kWaveformGroup, kStemReorderOnChangeKey, bool, waveformStemReorderOnChange, true);
void QmlConfigProxy::set_waveformStemReorderOnChange(bool value) {
    if (WaveformWidgetFactory::isCreated()) {
        WaveformWidgetFactory::instance()->setStemReorderOnChange(value);
        emit waveformStemReorderOnChangeChanged();
        return;
    }
    setConfigValueAndNotify<bool>(kWaveformGroup,
            kStemReorderOnChangeKey,
            value,
            true,
            &QmlConfigProxy::waveformStemReorderOnChangeChanged);
}
bool QmlConfigProxy::waveformStemSplitTracks() const {
    if (WaveformWidgetFactory::isCreated()) {
        return WaveformWidgetFactory::instance()->isStemSplitTracks();
    }
    return m_pConfig->getValue<bool>(ConfigKey(kWaveformGroup, kStemSplitTracksKey), false);
}

void QmlConfigProxy::set_waveformStemSplitTracks(bool value) {
    if (WaveformWidgetFactory::isCreated()) {
        WaveformWidgetFactory::instance()->setStemSplitTracks(value);
        emit waveformStemSplitTracksChanged();
        return;
    }
    if (value) {
        m_pConfig->setValue(ConfigKey(kWaveformGroup, kStemSplitTracksKey), value);
    } else {
        m_pConfig->remove(ConfigKey(kWaveformGroup, kStemSplitTracksKey));
    }
    emit waveformStemSplitTracksChanged();
}
PROPERTY_IMPL(kLibraryGroup,
        kWaveformCachingEnabledKey,
        bool,
        waveformCachingEnabled,
        true);
PROPERTY_IMPL(kLibraryGroup,
        kWaveformGenerationWithAnalysisEnabledKey,
        bool,
        waveformGenerationWithAnalysisEnabled,
        true);

double QmlConfigProxy::waveformAverageFrameRate() const {
    if (!WaveformWidgetFactory::isCreated()) {
        return 0.0;
    }
    return WaveformWidgetFactory::instance()->actualFrameRate();
}
PROPERTY_IMPL(kControlsGroup,
        kTooltipsKey,
        mixxx::preferences::Tooltips,
        libraryTooltips,
        mixxx::preferences::Tooltips::On);
PROPERTY_IMPL(kConfigGroup,
        kInhibitScreensaverKey,
        mixxx::preferences::ScreenSaver,
        libraryInhibitScreensaver,
        mixxx::preferences::ScreenSaver::On);
PROPERTY_IMPL(kConfigGroup, kHideMenuBarKey, bool, libraryHideMenuBar, false);
PROPERTY_IMPL(kLibraryGroup,
        kEnableSearchCompletionsKey,
        bool,
        libraryEnableSearchCompletions,
        true);
PROPERTY_IMPL(kLibraryGroup,
        kEnableSearchHistoryShortcutsKey,
        bool,
        librarySearchHistoryShortcutsEnable,
        true);
PROPERTY_IMPL_CONFIGKEY(mixxx::library::prefs::kSyncTrackMetadataConfigKey,
        bool,
        librarySyncTrackMetadataExport,
        false);
PROPERTY_IMPL_CONFIGKEY(mixxx::library::prefs::kSyncSeratoMetadataConfigKey,
        bool,
        librarySeratoMetadataExport,
        false);
PROPERTY_IMPL_CONFIGKEY(
        mixxx::library::prefs::kUseRelativePathOnExportConfigKey,
        bool,
        libraryUseRelativePathOnExport,
        false);
PROPERTY_IMPL_CONFIGKEY(mixxx::library::prefs::kHistoryMinTracksToKeepConfigKey,
        int,
        libraryHistoryMinTracksToKeep,
        1);
PROPERTY_IMPL_CONFIGKEY(
        mixxx::library::prefs::kHistoryTrackDuplicateDistanceConfigKey,
        int,
        libraryHistoryTrackDuplicateDistance,
        6);
PROPERTY_IMPL_CONFIGKEY(mixxx::library::prefs::kSearchBpmFuzzyRangeConfigKey,
        double,
        librarySearchBpmFuzzyRange,
        0.06);
PROPERTY_IMPL_CONFIGKEY(
        mixxx::library::prefs::kSearchDebouncingTimeoutMillisConfigKey,
        int,
        librarySearchDebouncingTimeout,
        300);
PROPERTY_IMPL_CONFIGKEY(
        mixxx::library::prefs::kEnableSearchCompletionsConfigKey,
        bool,
        librarySearchCompletionsEnable,
        true);
PROPERTY_IMPL_CONFIGKEY(
        mixxx::library::prefs::kEnableSearchHistoryShortcutsConfigKey,
        bool,
        libraryEnableSearchHistoryShortcuts,
        true);
PROPERTY_IMPL_CONFIGKEY(mixxx::library::prefs::kBpmColumnPrecisionConfigKey,
        int,
        libraryBpmColumnPrecision,
        BaseTrackTableModel::kBpmColumnPrecisionDefault);
PROPERTY_IMPL(kLibraryGroup, kRowHeightKey, double, libraryRowHeight, Library::kDefaultRowHeightPx);
PROPERTY_IMPL(kLibraryGroup, kRhythmboxEnabled, bool, libraryRhythmboxEnabled, false);
PROPERTY_IMPL(kLibraryGroup, kBansheeEnabled, bool, libraryBansheeEnabled, false);
PROPERTY_IMPL(kLibraryGroup, kITunesEnabled, bool, libraryITunesEnabled, false);
PROPERTY_IMPL(kLibraryGroup, kTraktorEnabled, bool, libraryTraktorEnabled, false);
PROPERTY_IMPL(kLibraryGroup, kRekordboxEnabled, bool, libraryRekordboxEnabled, false);
PROPERTY_IMPL(kLibraryGroup, kSeratoEnabled, bool, librarySeratoEnabled, false);

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
PROPERTY_IMPL(kControlsGroup,
        kTimeFormatKey,
        TrackTime::DisplayFormat,
        controlTimeFormat,
        TrackTime::DisplayFormat::TRADITIONAL);
PROPERTY_IMPL(kControlsGroup,
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
PROPERTY_IMPL(kConfigGroup,
        kKeyColorsEnabledKey,
        bool,
        configKeyColorsEnabled,
        BaseTrackTableModel::kKeyColorsEnabledDefault);
PROPERTY_IMPL(kConfigGroup,
        kStartInFullscreenKey,
        bool,
        configStartInFullscreenKey,
        false);
PROPERTY_IMPL(kConfigGroup,
        kSchemeKey,
        QString,
        configScheme,
        QStringLiteral("PaleMoon"));
PROPERTY_IMPL(kConfigGroup,
        kResizableSkinKey,
        QString,
        configSkin,
        QString());
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
