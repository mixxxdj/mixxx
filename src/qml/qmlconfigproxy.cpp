#include "qml/qmlconfigproxy.h"

#include <qglobal.h>

#include <map>

#include "library/basetracktablemodel.h"
#include "library/library.h"
#include "moc_qmlconfigproxy.cpp"
#include "preferences/colorpalettesettings.h"
#include "preferences/constants.h"
#include "util/color/predefinedcolorpalettes.h"
#include "waveform/widgets/waveformwidgettype.h"

#define PROPERTY_IMPL_GETTER(GROUP, KEY, TYPE, NAME, DEFAULT) \
    TYPE QmlConfigProxy::NAME() const {                       \
        return m_pConfig->getValue(                           \
                ConfigKey(GROUP, KEY),                        \
                DEFAULT);                                     \
    }

#define PROPERTY_IMPL(GROUP, KEY, TYPE, NAME, DEFAULT)    \
    PROPERTY_IMPL_GETTER(GROUP, KEY, TYPE, NAME, DEFAULT) \
    void QmlConfigProxy::set_##NAME(TYPE value) {         \
        m_pConfig->setValue(                              \
                ConfigKey(GROUP, KEY),                    \
                value);                                   \
        emit NAME##Changed();                             \
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
const QString kLibraryGroup = QStringLiteral("[Library]");
const QString kWaveformGroup = QStringLiteral("[Waveform]");

const QString kMultiSamplingKey = QStringLiteral("multi_sampling");
const QString k3DHardwareAccelerationKey = QStringLiteral("force_hardware_acceleration");

// Library group
const QString kSyncTrackMetadataExport = QStringLiteral("SyncTrackMetadataExport");
const QString kSeratoMetadataExport = QStringLiteral("SeratoMetadataExport");
const QString kUseRelativePathOnExport = QStringLiteral("UseRelativePathOnExport");
const QString kHistoryMinTracksToKeep = QStringLiteral("history_min_tracks_to_keep");
const QString kHistoryTrackDuplicateDistance = QStringLiteral("history_track_duplicate_distance");
const QString kSearchBpmFuzzyRange = QStringLiteral("search_bpm_fuzzy_range");
const QString kSearchDebouncingTimeout = QStringLiteral("SearchDebouncingTimeoutMillis");
const QString kRhythmboxEnabled = QStringLiteral("ShowRhythmboxLibrary");
const QString kBansheeEnabled = QStringLiteral("ShowBansheeLibrary");
const QString kITunesEnabled = QStringLiteral("ShowITunesLibrary");
const QString kTraktorEnabled = QStringLiteral("ShowTraktorLibrary");
const QString kRekordboxEnabled = QStringLiteral("ShowRekordboxLibrary");
const QString kSeratoEnabled = QStringLiteral("ShowSeratoLibrary");
const QString kEnableSearchCompletionsKey = QStringLiteral("EnableSearchCompletions");
const QString kEnableSearchHistoryShortcutsKey = QStringLiteral("EnableSearchHistoryShortcuts");

// Waveform group
const QString kWaveformZoomSynchronizationKey = QStringLiteral("ZoomSynchronization");
const QString kWaveformDefaultZoomKey = QStringLiteral("DefaultZoom");

const bool kWaveformZoomSynchronizationDefault = true;
const double kWaveformDefaultZoomDefault = 3.0;

} // namespace

namespace mixxx {
namespace qml {

QmlConfigProxy::QmlConfigProxy(
        UserSettingsPointer pConfig, QObject* parent)
        : QObject(parent), m_pConfig(pConfig) {
}

QVariantList QmlConfigProxy::getHotcueColorPalette() {
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    return paletteToQColorList(colorPaletteSettings.getHotcueColorPalette());
}

QVariantList QmlConfigProxy::getTrackColorPalette() {
    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    return paletteToQColorList(colorPaletteSettings.getTrackColorPalette());
}

int QmlConfigProxy::getMultiSamplingLevel() {
    return static_cast<int>(m_pConfig->getValue(
            ConfigKey(kPreferencesGroup, kMultiSamplingKey),
            mixxx::preferences::MultiSamplingMode::Disabled));
}

bool QmlConfigProxy::useAcceleration() {
    if (!m_pConfig->exists(
                ConfigKey(kPreferencesGroup, k3DHardwareAccelerationKey))) {
        // TODO: detect whether QML currently run with 3D acceleration. QSGRendererInterface?
        return false;
    }
    return m_pConfig->getValue<bool>(
            ConfigKey(kPreferencesGroup, k3DHardwareAccelerationKey));
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

PROPERTY_IMPL(kLibraryGroup, kSyncTrackMetadataExport, bool, librarySyncTrackMetadataExport, false);
PROPERTY_IMPL(kLibraryGroup, kSeratoMetadataExport, bool, librarySeratoMetadataExport, false);
PROPERTY_IMPL(kLibraryGroup, kUseRelativePathOnExport, bool, libraryUseRelativePathOnExport, false);
PROPERTY_IMPL(kLibraryGroup, kHistoryMinTracksToKeep, int, libraryHistoryMinTracksToKeep, 1);
PROPERTY_IMPL(kLibraryGroup,
        kHistoryTrackDuplicateDistance,
        int,
        libraryHistoryTrackDuplicateDistance,
        6);
PROPERTY_IMPL(kLibraryGroup, kSearchBpmFuzzyRange, double, librarySearchBpmFuzzyRange, 0.06);
PROPERTY_IMPL(kLibraryGroup, kSearchDebouncingTimeout, int, librarySearchDebouncingTimeout, 300);
PROPERTY_IMPL(kLibraryGroup,
        kEnableSearchCompletionsKey,
        bool,
        librarySearchCompletionsEnable,
        true);
PROPERTY_IMPL(kLibraryGroup,
        kEnableSearchHistoryShortcutsKey,
        bool,
        librarySearchHistoryShortcutsEnable,
        true);

PROPERTY_IMPL(kLibraryGroup, kRhythmboxEnabled, bool, libraryRhythmboxEnabled, false);
PROPERTY_IMPL(kLibraryGroup, kBansheeEnabled, bool, libraryBansheeEnabled, false);
PROPERTY_IMPL(kLibraryGroup, kITunesEnabled, bool, libraryITunesEnabled, false);
PROPERTY_IMPL(kLibraryGroup, kTraktorEnabled, bool, libraryTraktorEnabled, false);
PROPERTY_IMPL(kLibraryGroup, kRekordboxEnabled, bool, libraryRekordboxEnabled, false);
PROPERTY_IMPL(kLibraryGroup, kSeratoEnabled, bool, librarySeratoEnabled, false);

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
