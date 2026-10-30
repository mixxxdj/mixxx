#include "preferences/dialog/dlgprefwaveform.h"

#include <QColorDialog>
#include <QLocale>
#include <QMetaEnum>
#include <QtGlobal>
#ifdef MIXXX_USE_QML
#include <QQuickWindow>
#include <QSGRendererInterface>
#endif

#include "control/controlpushbutton.h"
#include "library/dao/analysisdao.h"
#include "library/library.h"
#include "moc_dlgprefwaveform.cpp"
#include "preferences/waveformsettings.h"
#include "util/cmdlineargs.h"
#include "util/db/dbconnectionpooled.h"
#include "waveform/overviewtype.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveformwidgetfactory.h"
#ifdef MIXXX_USE_QML
#include "qml/qmlconfigproxy.h"
#endif

namespace {
const QString kWaveformGroup(QStringLiteral("[Waveform]"));
const ConfigKey kOverviewTypeCfgKey(kWaveformGroup,
        QStringLiteral("WaveformOverviewType"));

#ifdef MIXXX_USE_QML
QString quickGraphicsApiName() {
    switch (QQuickWindow::graphicsApi()) {
    case QSGRendererInterface::OpenGL:
        return QStringLiteral("OpenGL");
    case QSGRendererInterface::Metal:
        return QStringLiteral("Metal");
    case QSGRendererInterface::Direct3D11:
        return QStringLiteral("Direct3D 11");
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    case QSGRendererInterface::Direct3D12:
        return QStringLiteral("Direct3D 12");
#endif
    case QSGRendererInterface::Vulkan:
        return QStringLiteral("Vulkan");
    case QSGRendererInterface::Software:
        return QStringLiteral("Software");
    case QSGRendererInterface::Null:
        return QStringLiteral("Null");
    default:
        return QStringLiteral("Unknown");
    }
}

bool isQtQuickWaveformRenderActive() {
    return WaveformWidgetFactory::isQmlMode();
}
#endif
} // namespace

// for OverviewType
using namespace mixxx;

DlgPrefWaveform::DlgPrefWaveform(
        QWidget* pParent,
        UserSettingsPointer pConfig,
        std::shared_ptr<Library> pLibrary)
        : DlgPreferencePage(pParent),
          m_pConfig(pConfig),
          m_pLibrary(pLibrary) {
    setupUi(this);
    frameRateAverage->setText(tr("Measuring…"));

#ifdef MIXXX_USE_QML
    if (isQtQuickWaveformRenderActive()) {
        // Qt Quick owns scene-graph frame scheduling, so the legacy QWidget
        // frame-rate controls do not apply to QML waveforms.
        frameRateLabel->setVisible(false);
        frameRateSlider->setVisible(false);
        frameRateSpinBox->setVisible(false);
    }
#endif

    // Waveform overview init
    waveformOverviewComboBox->addItem(
            tr("Filtered"), QVariant::fromValue(OverviewType::Filtered));
    waveformOverviewComboBox->addItem(tr("HSV"), QVariant::fromValue(OverviewType::HSV));
    waveformOverviewComboBox->addItem(tr("RGB"), QVariant::fromValue(OverviewType::RGB));
    if (!ControlObject::exists(kOverviewTypeCfgKey)) {
        m_pTypeControl = std::make_unique<ControlPushButton>(kOverviewTypeCfgKey);
        m_pTypeControl->setStates(QMetaEnum::fromType<OverviewType>().keyCount());
        m_pTypeControl->setReadOnly();
    }
    // Update the control with the config value
    OverviewType overviewType =
            m_pConfig->getValue<OverviewType>(kOverviewTypeCfgKey, OverviewType::RGB);
    int cfgTypeIndex = waveformOverviewComboBox->findData(QVariant::fromValue(overviewType));
    if (cfgTypeIndex == -1) {
        // Invalid config value, set default type RGB and write it to config
        cfgTypeIndex = waveformOverviewComboBox->findData(
                QVariant::fromValue(OverviewType::RGB));
        waveformOverviewComboBox->setCurrentIndex(cfgTypeIndex);
        m_pConfig->setValue(kOverviewTypeCfgKey, cfgTypeIndex);
    } else {
        waveformOverviewComboBox->setCurrentIndex(cfgTypeIndex);
    }
    // Set the control used by WOverview
    if (m_pTypeControl) {
        m_pTypeControl->forceSet(cfgTypeIndex);
    } else if (auto* pControl = ControlObject::getControl(kOverviewTypeCfgKey)) {
        pControl->forceSet(cfgTypeIndex);
    }

    // Populate waveform options.
    auto* pFactory = WaveformWidgetFactory::instance();
    // We assume that the original type list order remains constant.
    // We will use the type index later on to set waveform types and to
    // update the combobox.
    QVector<WaveformWidgetAbstractHandle> types = pFactory->getAvailableTypes();
    for (int i = 0; i < types.size(); ++i) {
        if (types[i].getType() == WaveformWidgetType::Empty) {
            continue;
        }
        waveformTypeComboBox->addItem(types[i].getDisplayName(), types[i].getType());
    }
    // Sort the combobox items alphabetically
    waveformTypeComboBox->model()->sort(0);

    // Populate zoom options.
    for (int i = static_cast<int>(WaveformWidgetRenderer::s_waveformMinZoom);
            i <= static_cast<int>(WaveformWidgetRenderer::s_waveformMaxZoom);
            i++) {
        defaultZoomComboBox->addItem(QString::number(100 / static_cast<double>(i), 'f', 1) + " %");
    }

    const ConfigKey overviewStereoKey(
            kWaveformGroup, QStringLiteral("overview_stereo_mode"));
    if (!ControlObject::exists(overviewStereoKey)) {
        m_pOverviewStereoControl = std::make_unique<ControlObject>(overviewStereoKey);
        m_pOverviewStereoControl->setReadOnly();
    }

    const ConfigKey overviewMinuteMarkersKey(
            kWaveformGroup, QStringLiteral("draw_overview_minute_markers"));
    if (!ControlObject::exists(overviewMinuteMarkersKey)) {
        m_pOverviewMinuteMarkersControl = std::make_unique<ControlObject>(
                overviewMinuteMarkersKey);
        m_pOverviewMinuteMarkersControl->setReadOnly();
    }

    // Populate untilMark options
    untilMarkAlignComboBox->addItem(tr("Top"));
    untilMarkAlignComboBox->addItem(tr("Center"));
    untilMarkAlignComboBox->addItem(tr("Bottom"));

    //: options for "Text height limit"
    untilMarkTextHeightLimitComboBox->addItem(tr("1/3 of waveform viewer"));
    untilMarkTextHeightLimitComboBox->addItem(tr("Entire waveform viewer"));

    stemDisplayModeComboBox->addItem(tr("Overlapping"));
    stemDisplayModeComboBox->addItem(tr("Stacked"));

    // Adopt tr string from first GLSL hint
    requiresGLSLLabel2->setText(requiresGLSLLabel->text());

    // Index matches WaveformMark::MemoryCueMarkerView ("MemoryCueMarkerShowIn")
    memoryCueMarkerViewComboBox->addItem(tr("Scrolling waveforms and overview"));
    memoryCueMarkerViewComboBox->addItem(tr("Scrolling waveforms only"));
    memoryCueMarkerViewComboBox->addItem(tr("Overview only"));
    // Index 0 = top, 1 = bottom, 2 = both (matches the "MemoryCueMarkerAlign" config value)
    memoryCueMarkerAlignComboBox->addItem(tr("Top"));
    memoryCueMarkerAlignComboBox->addItem(tr("Bottom"));
    memoryCueMarkerAlignComboBox->addItem(tr("Top and bottom"));
    // Index 0 = top, 1 = bottom (matches the "MemoryCueNameAlign" config value)
    memoryCueNameAlignComboBox->addItem(tr("Top"));
    memoryCueNameAlignComboBox->addItem(tr("Bottom"));
    // Index matches WaveformMark::MemoryCueNameBackground
    memoryCueNameBackgroundComboBox->addItem(tr("Off"));
    memoryCueNameBackgroundComboBox->addItem(tr("Cue color"));
    memoryCueNameBackgroundComboBox->addItem(tr("Custom color"));

    // The GUI is not fully setup so connecting signals before calling
    // slotUpdate can generate rebootMixxxView calls.
    // TODO(XXX): Improve this awkwardness.
    slotUpdate();

    connect(frameRateSpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgPrefWaveform::slotSetFrameRate);
    connect(endOfTrackWarningTimeSpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgPrefWaveform::slotSetWaveformEndRender);
    connect(beatGridAlphaSpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgPrefWaveform::slotSetBeatGridAlpha);
    connect(frameRateSlider,
            &QSlider::valueChanged,
            frameRateSpinBox,
            &QSpinBox::setValue);
    connect(frameRateSpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            frameRateSlider,
            &QSlider::setValue);
    connect(endOfTrackWarningTimeSlider,
            &QSlider::valueChanged,
            endOfTrackWarningTimeSpinBox,
            &QSpinBox::setValue);
    connect(endOfTrackWarningTimeSpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            endOfTrackWarningTimeSlider,
            &QSlider::setValue);
    connect(beatGridAlphaSlider,
            &QSlider::valueChanged,
            beatGridAlphaSpinBox,
            &QSpinBox::setValue);
    connect(beatGridAlphaSpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            beatGridAlphaSlider,
            &QSlider::setValue);

    connect(useWaveformCheckBox,
            &QCheckBox::clicked,
            this,
            &DlgPrefWaveform::slotSetWaveformEnabled);

    connect(waveformTypeComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefWaveform::slotSetWaveformType);

    connect(useAccelerationCheckBox,
            &QCheckBox::clicked,
            this,
            &DlgPrefWaveform::slotSetWaveformAcceleration);
    connect(splitLeftRightCheckBox,
            &QCheckBox::clicked,
            this,
            &DlgPrefWaveform::slotSetWaveformOptionSplitStereoSignal);
    connect(highDetailCheckBox,
            &QCheckBox::clicked,
            this,
            &DlgPrefWaveform::slotSetWaveformOptionHighDetail);
    connect(defaultZoomComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefWaveform::slotSetDefaultZoom);
    connect(synchronizeZoomCheckBox,
            &QCheckBox::clicked,
            this,
            &DlgPrefWaveform::slotSetZoomSynchronization);
    connect(allVisualGain,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &DlgPrefWaveform::slotSetVisualGainAll);
    connect(lowVisualGain,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &DlgPrefWaveform::slotSetVisualGainLow);
    connect(midVisualGain,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &DlgPrefWaveform::slotSetVisualGainMid);
    connect(highVisualGain,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &DlgPrefWaveform::slotSetVisualGainHigh);

    connect(overview_scale_options,
            QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked),
            this,
            &DlgPrefWaveform::slotSetOverviewScaling);
    connect(overviewMinuteMarkersCheckBox,
            &QCheckBox::toggled,
            this,
            &DlgPrefWaveform::slotSetOverviewMinuteMarkers);
    connect(overviewStereoCheckBox,
            &QCheckBox::toggled,
            this,
            &DlgPrefWaveform::slotSetOverviewStereoMode);

    connect(pFactory,
            &WaveformWidgetFactory::waveformMeasured,
            this,
            &DlgPrefWaveform::slotWaveformMeasured);
    connect(waveformOverviewComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefWaveform::slotSetWaveformOverviewType);
    connect(clearCachedWaveforms,
            &QAbstractButton::clicked,
            this,
            &DlgPrefWaveform::slotClearCachedWaveforms);
    connect(playMarkerPositionSlider,
            &QSlider::valueChanged,
            this,
            &DlgPrefWaveform::slotSetPlayMarkerPosition);
    connect(untilMarkShowBeatsCheckBox,
            &QCheckBox::toggled,
            this,
            &DlgPrefWaveform::slotSetUntilMarkShowBeats);
    connect(showMemoryCueMarkersCheckBox,
            &QCheckBox::toggled,
            this,
            &DlgPrefWaveform::slotSetShowMemoryCueMarkers);
    connect(memoryCueMarkerViewComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefWaveform::slotSetMemoryCueMarkerView);
    connect(memoryCueMarkerAlignComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefWaveform::slotSetMemoryCueMarkerAlign);
    connect(memoryCueMarkerLineWidthSpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgPrefWaveform::slotSetMemoryCueMarkerLineWidth);
    connect(memoryCueMarkerOpacitySpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgPrefWaveform::slotSetMemoryCueMarkerOpacity);
    connect(memoryCueMarkerSizeSpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgPrefWaveform::slotSetMemoryCueMarkerSize);
    connect(memoryCueMarkerCustomOutlineCheckBox,
            &QCheckBox::toggled,
            this,
            &DlgPrefWaveform::slotSetMemoryCueMarkerCustomOutline);
    connect(memoryCueMarkerOutlineColorButton,
            &QPushButton::clicked,
            this,
            &DlgPrefWaveform::slotChooseMemoryCueMarkerOutlineColor);
    connect(memoryCueMarkerSingleColorCheckBox,
            &QCheckBox::toggled,
            this,
            &DlgPrefWaveform::slotSetMemoryCueMarkerSingleColor);
    connect(memoryCueMarkerColorButton,
            &QPushButton::clicked,
            this,
            &DlgPrefWaveform::slotChooseMemoryCueMarkerColor);
    connect(memoryCueMarkerShowNamesCheckBox,
            &QCheckBox::toggled,
            this,
            &DlgPrefWaveform::slotSetMemoryCueMarkerShowNames);
    connect(memoryCueNameAlignComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefWaveform::slotSetMemoryCueNameAlign);
    connect(memoryCueNameBackgroundComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefWaveform::slotSetMemoryCueNameBackground);
    connect(memoryCueNameBackgroundColorButton,
            &QPushButton::clicked,
            this,
            &DlgPrefWaveform::slotChooseMemoryCueNameBackgroundColor);
    connect(memoryCueNameCustomTextColorCheckBox,
            &QCheckBox::toggled,
            this,
            &DlgPrefWaveform::slotSetMemoryCueNameCustomTextColor);
    connect(memoryCueNameTextColorButton,
            &QPushButton::clicked,
            this,
            &DlgPrefWaveform::slotChooseMemoryCueNameTextColor);
    connect(untilMarkShowTimeCheckBox,
            &QCheckBox::toggled,
            this,
            &DlgPrefWaveform::slotSetUntilMarkShowTime);
    connect(untilMarkAlignComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefWaveform::slotSetUntilMarkAlign);
    connect(untilMarkTextPointSizeSpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgPrefWaveform::slotSetUntilMarkTextPointSize);
    connect(untilMarkTextHeightLimitComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefWaveform::slotSetUntilMarkTextHeightLimit);
    connect(stemReorderLayerOnChangedCheckBox,
            &QCheckBox::clicked,
            this,
            &DlgPrefWaveform::slotStemReorderOnChange);
    connect(stemOpacitySpinBox,
            &QDoubleSpinBox::valueChanged,
            this,
            &DlgPrefWaveform::slotStemOpacity);
    connect(stemOutlineOpacitySpinBox,
            &QDoubleSpinBox::valueChanged,
            this,
            &DlgPrefWaveform::slotStemOutlineOpacity);
    connect(stemDisplayModeComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefWaveform::slotStemDisplayMode);

    setScrollSafeGuardForAllInputWidgets(this);
}

void DlgPrefWaveform::slotSetWaveformOptions(
        allshader::WaveformRendererSignalBase::Option option, bool enabled) {
    auto* pFactory = WaveformWidgetFactory::instance();
    auto type = static_cast<WaveformWidgetType::Type>(
            waveformTypeComboBox->currentData().toInt());
    pFactory->setWaveformOption(option, enabled, type);
    pFactory->setWidgetTypeFromHandle(
            pFactory->findHandleIndexFromType(type), true);
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotUpdate() {
    auto* pFactory = WaveformWidgetFactory::instance();

#ifdef MIXXX_USE_QML
    if (isQtQuickWaveformRenderActive()) {
        // The QML settings page writes directly through QmlConfigProxy. Reload the
        // shared factory before presenting the legacy page so both preference
        // surfaces reflect the same persisted values.
        pFactory->setConfig(m_pConfig);
        pFactory->setWidgetTypeFromConfig();
        graphicsApiStatusData->setText(
                tr("Qt Quick renderer: %1").arg(quickGraphicsApiName()));
        // QML chooses acceleration through the scene-graph backend. The
        // legacy per-waveform acceleration switch does not apply here.
        useAccelerationCheckBox->setEnabled(false);
        useAccelerationCheckBox->setChecked(true);
    } else
#endif

            if (pFactory->isOpenGlAvailable() || pFactory->isOpenGlesAvailable()) {
        graphicsApiStatusData->setText(pFactory->getOpenGLVersion());
        useAccelerationCheckBox->setEnabled(true);
        bool isAccelerationEnabled =
                pFactory->getBackendFromConfig() != WaveformWidgetBackend::None;
        useAccelerationCheckBox->setChecked(isAccelerationEnabled);
    } else {
        graphicsApiStatusData->setText(tr("OpenGL not available") + ": " +
                pFactory->getOpenGLVersion());
        useAccelerationCheckBox->setEnabled(false);
        useAccelerationCheckBox->setChecked(false);
    }

    // The combobox holds a list of [handle name, handle index]
    int indexOfCurrentType = waveformTypeComboBox->findData(pFactory->getType());
    if (indexOfCurrentType != -1 && waveformTypeComboBox->currentIndex() != indexOfCurrentType) {
        waveformTypeComboBox->setCurrentIndex(indexOfCurrentType);
    }

    auto type = pFactory->getType();
    bool useWaveform = type != WaveformWidgetType::Empty;
    useWaveformCheckBox->setChecked(useWaveform);

    updateWaveformAcceleration(type);
    updateWaveformTypeOptions(useWaveform);
#ifdef MIXXX_USE_QML
    if (isQtQuickWaveformRenderActive()) {
        // The scene-graph renderer currently ignores the legacy textured/high
        // detail option. Keep the control visible for parity, but make its
        // unsupported state explicit.
        highDetailCheckBox->setEnabled(false);
        highDetailCheckBox->setChecked(false);
    }
#endif
    waveformTypeComboBox->setEnabled(useWaveform);
    updateEnableUntilMark();
    updateWaveformGeneralOptionsEnabled();
    updateStemOptionsEnabled();

    frameRateSpinBox->setValue(pFactory->getFrameRate());
    frameRateSlider->setValue(pFactory->getFrameRate());
    endOfTrackWarningTimeSpinBox->setValue(pFactory->getEndOfTrackWarningTime());
    endOfTrackWarningTimeSlider->setValue(pFactory->getEndOfTrackWarningTime());
    synchronizeZoomCheckBox->setChecked(pFactory->isZoomSync());
    allVisualGain->setValue(pFactory->getVisualGain(BandIndex::AllBand));
    lowVisualGain->setValue(pFactory->getVisualGain(BandIndex::Low));
    midVisualGain->setValue(pFactory->getVisualGain(BandIndex::Mid));
    highVisualGain->setValue(pFactory->getVisualGain(BandIndex::High));
    // Round zoom to int to get a default zoom index.
    defaultZoomComboBox->setCurrentIndex(static_cast<int>(pFactory->getDefaultZoom()) - 1);
    playMarkerPositionSlider->setValue(static_cast<int>(pFactory->getPlayMarkerPosition() * 100));
    beatGridAlphaSpinBox->setValue(pFactory->getBeatGridAlpha());
    beatGridAlphaSlider->setValue(pFactory->getBeatGridAlpha());

    showMemoryCueMarkersCheckBox->setChecked(pFactory->getShowMemoryCueMarkers());
    memoryCueMarkerViewComboBox->setCurrentIndex(
            static_cast<int>(pFactory->getMemoryCueMarkerView()));
    {
        const Qt::Alignment memoryCueAlign = pFactory->getMemoryCueMarkerAlign();
        const bool top = memoryCueAlign.testFlag(Qt::AlignTop);
        const bool bottom = memoryCueAlign.testFlag(Qt::AlignBottom);
        memoryCueMarkerAlignComboBox->setCurrentIndex(top && bottom ? 2 : (top ? 0 : 1));
    }
    memoryCueMarkerLineWidthSpinBox->setValue(pFactory->getMemoryCueMarkerLineWidth());
    memoryCueMarkerSizeSpinBox->setValue(pFactory->getMemoryCueMarkerSize());
    memoryCueMarkerOpacitySpinBox->setValue(pFactory->getMemoryCueMarkerOpacity());
    memoryCueMarkerCustomOutlineCheckBox->setChecked(
            pFactory->getMemoryCueMarkerCustomOutline());
    updateMemoryCueMarkerOutlineColorButton();
    memoryCueMarkerSingleColorCheckBox->setChecked(pFactory->getMemoryCueMarkerSingleColor());
    updateMemoryCueMarkerColorButton();
    memoryCueMarkerShowNamesCheckBox->setChecked(pFactory->getMemoryCueMarkerShowNames());
    memoryCueNameAlignComboBox->setCurrentIndex(
            pFactory->getMemoryCueNameAlign().testFlag(Qt::AlignBottom) ? 1 : 0);
    memoryCueNameBackgroundComboBox->setCurrentIndex(
            static_cast<int>(pFactory->getMemoryCueNameBackground()));
    updateMemoryCueNameBackgroundColorButton();
    memoryCueNameCustomTextColorCheckBox->setChecked(
            pFactory->getMemoryCueNameCustomTextColor());
    updateMemoryCueNameTextColorButton();
    updateEnableMemoryCueMarkers();
    untilMarkShowBeatsCheckBox->setChecked(pFactory->getUntilMarkShowBeats());
    untilMarkShowTimeCheckBox->setChecked(pFactory->getUntilMarkShowTime());
    untilMarkAlignComboBox->setCurrentIndex(
            WaveformWidgetFactory::toUntilMarkAlignIndex(
                    pFactory->getUntilMarkAlign()));
    untilMarkTextPointSizeSpinBox->setValue(pFactory->getUntilMarkTextPointSize());
    untilMarkTextHeightLimitComboBox->setCurrentIndex(
            WaveformWidgetFactory::toUntilMarkTextHeightLimitIndex(
                    pFactory->getUntilMarkTextHeightLimit()));

    stemReorderLayerOnChangedCheckBox->setChecked(pFactory->isStemReorderOnChange());
    stemOpacitySpinBox->setValue(pFactory->getStemOpacity());
    stemOutlineOpacitySpinBox->setValue(pFactory->getStemOutlineOpacity());
    stemDisplayModeComboBox->setCurrentIndex(pFactory->isStemSplitTracks() ? 1 : 0);

    OverviewType cfgOverviewType =
            m_pConfig->getValue<OverviewType>(kOverviewTypeCfgKey, OverviewType::RGB);
    // Assumes the combobox index is in sync with the ControlPushButton
    if (cfgOverviewType != waveformOverviewComboBox->currentData().value<OverviewType>()) {
        int cfgOverviewTypeIndex =
                waveformOverviewComboBox->findData(QVariant::fromValue(cfgOverviewType));
        waveformOverviewComboBox->setCurrentIndex(cfgOverviewTypeIndex);
    }

    if (pFactory->isOverviewNormalized()) {
        overview_scale_normalize->setChecked(true);
    } else {
        overview_scale_allReplayGain->setChecked(true);
    }

    bool overviewStereo = m_pConfig->getValue(
            ConfigKey(kWaveformGroup, QStringLiteral("overview_stereo_mode")), true);
    overviewStereoCheckBox->setChecked(overviewStereo);
    if (m_pOverviewStereoControl) {
        m_pOverviewStereoControl->forceSet(overviewStereo);
    } else if (auto* pControl = ControlObject::getControl(
                       ConfigKey(kWaveformGroup, QStringLiteral("overview_stereo_mode")))) {
        pControl->forceSet(overviewStereo);
    }

    bool drawOverviewMinuteMarkers = m_pConfig->getValue(
            ConfigKey(kWaveformGroup, QStringLiteral("draw_overview_minute_markers")), true);
    overviewMinuteMarkersCheckBox->setChecked(drawOverviewMinuteMarkers);
    if (m_pOverviewMinuteMarkersControl) {
        m_pOverviewMinuteMarkersControl->forceSet(drawOverviewMinuteMarkers);
    } else if (auto* pControl = ControlObject::getControl(
                       ConfigKey(kWaveformGroup, QStringLiteral("draw_overview_minute_markers")))) {
        pControl->forceSet(drawOverviewMinuteMarkers);
    }

    WaveformSettings waveformSettings(m_pConfig);
    enableWaveformCaching->setChecked(waveformSettings.waveformCachingEnabled());
    enableWaveformGenerationWithAnalysis->setChecked(
            waveformSettings.waveformGenerationWithAnalysisEnabled());
    calculateCachedWaveformDiskUsage();
}

void DlgPrefWaveform::slotApply() {
    // All other settings have already been applied instantly for preview purpose
    WaveformSettings waveformSettings(m_pConfig);
    waveformSettings.setWaveformCachingEnabled(enableWaveformCaching->isChecked());
    waveformSettings.setWaveformGenerationWithAnalysisEnabled(
            enableWaveformGenerationWithAnalysis->isChecked());
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotResetToDefaults() {
    auto* pFactory = WaveformWidgetFactory::instance();

    int defaultIndex = waveformTypeComboBox->findData(
            WaveformWidgetFactory::defaultType());
    if (defaultIndex != -1 && waveformTypeComboBox->currentIndex() != defaultIndex) {
        waveformTypeComboBox->setCurrentIndex(defaultIndex);
    }
    pFactory->setDefaultBackend();
    useWaveformCheckBox->setChecked(true);
    waveformTypeComboBox->setEnabled(true);
    updateWaveformAcceleration(pFactory->defaultType());

    // Restore waveform backend and option setting instantly
    pFactory->resetWaveformOptions();
    updateWaveformTypeOptions(true);
    pFactory->setWidgetTypeFromHandle(
            pFactory->findHandleIndexFromType(
                    WaveformWidgetFactory::defaultType()),
            true);

    allVisualGain->setValue(WaveformWidgetFactory::getVisualGainDefault(BandIndex::AllBand));
    lowVisualGain->setValue(WaveformWidgetFactory::getVisualGainDefault(BandIndex::Low));
    midVisualGain->setValue(WaveformWidgetFactory::getVisualGainDefault(BandIndex::Mid));
    highVisualGain->setValue(WaveformWidgetFactory::getVisualGainDefault(BandIndex::High));

    // Default zoom level is 3 in WaveformWidgetFactory.
    defaultZoomComboBox->setCurrentIndex(3 + 1);

    synchronizeZoomCheckBox->setChecked(true);

    // RGB overview.
    waveformOverviewComboBox->setCurrentIndex(
            waveformOverviewComboBox->findData(QVariant::fromValue(OverviewType::RGB)));

    // Default to overview stereo mode
    overviewStereoCheckBox->setChecked(true);

    // Show minute markers.
    overviewMinuteMarkersCheckBox->setChecked(true);

    // Use "Global" waveform gain + ReplayGain if enabled
    overview_scale_allReplayGain->setChecked(!WaveformWidgetFactory::isOverviewNormalizedDefault());

    // 60FPS is the default
    frameRateSlider->setValue(60);
    endOfTrackWarningTimeSlider->setValue(30);

    // Waveform caching enabled.
    enableWaveformCaching->setChecked(true);
    enableWaveformGenerationWithAnalysis->setChecked(false);

    // Beat grid alpha default is 90
    beatGridAlphaSlider->setValue(90);

    showMemoryCueMarkersCheckBox->setChecked(true);
    memoryCueMarkerViewComboBox->setCurrentIndex(
            static_cast<int>(WaveformMark::MemoryCueMarkerView::Both));
    memoryCueMarkerAlignComboBox->setCurrentIndex(1); // Bottom
    memoryCueMarkerLineWidthSpinBox->setValue(1);
    memoryCueMarkerSizeSpinBox->setValue(11);
    memoryCueMarkerOpacitySpinBox->setValue(100);
    memoryCueMarkerCustomOutlineCheckBox->setChecked(false);
    WaveformWidgetFactory::instance()->setMemoryCueMarkerOutlineColor(Qt::white);
    updateMemoryCueMarkerOutlineColorButton();
    memoryCueMarkerSingleColorCheckBox->setChecked(false);
    WaveformWidgetFactory::instance()->setMemoryCueMarkerColor(QColor(230, 40, 40));
    updateMemoryCueMarkerColorButton();
    memoryCueMarkerShowNamesCheckBox->setChecked(false);
    memoryCueNameAlignComboBox->setCurrentIndex(0); // Top
    memoryCueNameBackgroundComboBox->setCurrentIndex(
            static_cast<int>(WaveformMark::MemoryCueNameBackground::CueColor));
    WaveformWidgetFactory::instance()->setMemoryCueNameBackgroundCustomColor(
            QColor(0, 0, 0, 153));
    updateMemoryCueNameBackgroundColorButton();
    memoryCueNameCustomTextColorCheckBox->setChecked(false);
    WaveformWidgetFactory::instance()->setMemoryCueNameTextColor(Qt::white);
    updateMemoryCueNameTextColorButton();
    beatGridAlphaSpinBox->setValue(90);

    // 50 (center) is default
    playMarkerPositionSlider->setValue(50);

    stemDisplayModeComboBox->setCurrentIndex(0);
}

void DlgPrefWaveform::notifyQmlWaveformSettingsChanged() {
#ifdef MIXXX_USE_QML
    if (isQtQuickWaveformRenderActive()) {
        mixxx::qml::QmlConfigProxy::notifyWaveformSettingsChanged();
    }
#endif
}

void DlgPrefWaveform::slotSetFrameRate(int frameRate) {
    WaveformWidgetFactory::instance()->setFrameRate(frameRate);
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotSetWaveformEndRender(int endTime) {
    WaveformWidgetFactory::instance()->setEndOfTrackWarningTime(endTime);
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotSetWaveformType(int index) {
    // Ignore sets for -1 since this happens when we clear the combobox.
    if (index < 0) {
        return;
    }
    auto type = static_cast<WaveformWidgetType::Type>(
            waveformTypeComboBox->itemData(index).toInt());
    auto* pFactory = WaveformWidgetFactory::instance();

    // When setting the type, factory uses current 'use acceleration' state,
    // which may currently be off. However, with QOpenGL there are Simple and Stacked
    // which require acceleration and auto-enable it if possible.
    // FIXME Find a better solution?
    // See https://github.com/mixxxdj/mixxx/pull/15277 for details.
    updateWaveformAcceleration(type);
    // Store the value so it's available in factory. Same as
    // slotSetWaveformAcceleration(useAccelerationCheckBox->isChecked()) just
    // without the redundant actions
    pFactory->setAcceleration(useAccelerationCheckBox->isChecked());

    // Now set the new type
    pFactory->setWidgetTypeFromHandle(pFactory->findHandleIndexFromType(type));

    updateWaveformTypeOptions(true);
    updateEnableUntilMark();
    updateStemOptionsEnabled();
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotSetWaveformEnabled(bool checked) {
    auto* pFactory = WaveformWidgetFactory::instance();
    if (!checked) {
        pFactory->setWidgetTypeFromHandle(
                pFactory->findHandleIndexFromType(WaveformWidgetType::Empty),
                true);
    } else {
        auto type = static_cast<WaveformWidgetType::Type>(
                waveformTypeComboBox->currentData().toInt());
        pFactory->setWidgetTypeFromHandle(pFactory->findHandleIndexFromType(type), true);
    }
    slotUpdate();
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotSetWaveformAcceleration(bool checked) {
    auto* pFactory = WaveformWidgetFactory::instance();
    pFactory->setAcceleration(checked);
    auto type = static_cast<WaveformWidgetType::Type>(waveformTypeComboBox->currentData().toInt());
    pFactory->setWidgetTypeFromHandle(pFactory->findHandleIndexFromType(type), true);
    updateWaveformTypeOptions(true);
    updateEnableUntilMark();
    updateStemOptionsEnabled();
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::updateWaveformAcceleration(WaveformWidgetType::Type type) {
    auto* pFactory = WaveformWidgetFactory::instance();
    auto backend = pFactory->getBackendFromConfig();

    bool supportAcceleration = pFactory->widgetTypeSupportsAcceleration(type);
    bool supportSoftware = pFactory->widgetTypeSupportsSoftware(type);

    useAccelerationCheckBox->blockSignals(true);

#ifdef MIXXX_USE_QML
    if (isQtQuickWaveformRenderActive()) {
        useAccelerationCheckBox->setChecked(type != WaveformWidgetType::Empty);
        useAccelerationCheckBox->setEnabled(false);
        useAccelerationCheckBox->blockSignals(false);
        return;
    }
#endif

    if (type == WaveformWidgetType::Empty) {
        useAccelerationCheckBox->setChecked(false);
    } else if (supportSoftware ^ supportAcceleration) {
        useAccelerationCheckBox->setChecked(!supportSoftware || supportAcceleration);
    } else {
        useAccelerationCheckBox->setChecked(backend != WaveformWidgetBackend::None);
    }

    useAccelerationCheckBox->setEnabled(supportAcceleration &&
            supportSoftware && type != WaveformWidgetType::Empty);

    useAccelerationCheckBox->blockSignals(false);
}

void DlgPrefWaveform::updateWaveformTypeOptions(bool useWaveform) {
    splitLeftRightCheckBox->blockSignals(true);
    highDetailCheckBox->blockSignals(true);

#ifdef MIXXX_USE_QOPENGL
    auto* pFactory = WaveformWidgetFactory::instance();

    auto type = static_cast<WaveformWidgetType::Type>(waveformTypeComboBox->currentData().toInt());
#ifdef MIXXX_USE_QML
    const bool qmlMode = isQtQuickWaveformRenderActive();
#else
    constexpr bool qmlMode = false;
#endif
    WaveformWidgetBackend backend = pFactory->getBackendFromConfig();
    allshader::WaveformRendererSignalBase::Options currentOptions = pFactory->getWaveformOptions();
    allshader::WaveformRendererSignalBase::Options supportedOptions =
            pFactory->getWaveformOptionsSupportedByType(type, backend);

    splitLeftRightCheckBox->setEnabled(useWaveform &&
            (supportedOptions &
                    allshader::WaveformRendererSignalBase::Option::SplitStereoSignal));
    highDetailCheckBox->setEnabled(useWaveform && !qmlMode &&
            (supportedOptions &
                    allshader::WaveformRendererSignalBase::Option::HighDetail));
    splitLeftRightCheckBox->setChecked(splitLeftRightCheckBox->isEnabled() &&
            (currentOptions &
                    allshader::WaveformRendererSignalBase::Option::SplitStereoSignal));
    highDetailCheckBox->setChecked(highDetailCheckBox->isEnabled() &&
            (currentOptions & allshader::WaveformRendererSignalBase::Option::HighDetail));
#else
    splitLeftRightCheckBox->setVisible(false);
    highDetailCheckBox->setVisible(false);
#endif

    splitLeftRightCheckBox->blockSignals(false);
    highDetailCheckBox->blockSignals(false);
}

void DlgPrefWaveform::updateEnableUntilMark() {
#ifndef MIXXX_USE_QOPENGL
    const bool enabled = false;
#else
    auto* pFactory = WaveformWidgetFactory::instance();
    const bool enabled =
            pFactory->widgetTypeSupportsUntilMark() &&
            pFactory->getBackendFromConfig() != WaveformWidgetBackend::None;
#endif
    untilMarkShowBeatsCheckBox->setEnabled(enabled);
    untilMarkShowTimeCheckBox->setEnabled(enabled);
    // Disable the beats/time options if neither beats nor time is enabled
    bool beatsOrTimeEnabled = untilMarkShowBeatsCheckBox->isChecked() ||
            untilMarkShowTimeCheckBox->isChecked();
    untilMarkAlignLabel->setEnabled(beatsOrTimeEnabled);
    untilMarkAlignComboBox->setEnabled(beatsOrTimeEnabled);
    untilMarkTextPointSizeLabel->setEnabled(beatsOrTimeEnabled);
    untilMarkTextPointSizeSpinBox->setEnabled(beatsOrTimeEnabled);
    untilMarkTextHeightLimitLabel->setEnabled(beatsOrTimeEnabled);
    untilMarkTextHeightLimitComboBox->setEnabled(beatsOrTimeEnabled);
    requiresGLSLLabel->setVisible(!enabled && useWaveformCheckBox->isChecked());
}

void DlgPrefWaveform::updateWaveformGeneralOptionsEnabled() {
    bool enabled = useWaveformCheckBox->isChecked();
    frameRateSlider->setEnabled(enabled);
    frameRateSpinBox->setEnabled(enabled);
    endOfTrackWarningTimeSlider->setEnabled(enabled);
    endOfTrackWarningTimeSpinBox->setEnabled(enabled);
    beatGridAlphaSlider->setEnabled(enabled);
    beatGridAlphaSpinBox->setEnabled(enabled);
    playMarkerPositionSlider->setEnabled(enabled);
    defaultZoomComboBox->setEnabled(enabled);
    synchronizeZoomCheckBox->setEnabled(enabled);
    updateWaveformGainEnabled();
    updateStemOptionsEnabled();
}

void DlgPrefWaveform::updateStemOptionsEnabled() {
#ifndef MIXXX_USE_QOPENGL
    const bool stemsSupported = false;
#else
    auto* pFactory = WaveformWidgetFactory::instance();
    const bool stemsSupported =
            pFactory->widgetTypeSupportsStems() &&
            pFactory->getBackendFromConfig() == WaveformWidgetBackend::AllShader;
#endif
    bool enabled = useWaveformCheckBox->isChecked();
    const bool stemReorderEnabled = stemsSupported && enabled &&
            stemDisplayModeComboBox->currentIndex() == 0;
    stemOpacityMainLabel->setEnabled(stemsSupported && enabled);
    stemOpacityOutlineLabel->setEnabled(stemsSupported && enabled);
    stemDisplayModeLabel->setEnabled(stemsSupported && enabled);
    stemReorderLayerOnChangedCheckBox->setEnabled(stemReorderEnabled);
    stemOpacitySpinBox->setEnabled(stemsSupported && enabled);
    stemOutlineOpacitySpinBox->setEnabled(stemsSupported && enabled);
    stemDisplayModeComboBox->setEnabled(stemsSupported && enabled);
    requiresGLSLLabel2->setVisible(!stemsSupported && enabled);
}

void DlgPrefWaveform::updateWaveformGainEnabled() {
    bool waveformsEnabled = useWaveformCheckBox->isChecked();
    bool allGainEnabled = waveformsEnabled || overview_scale_allReplayGain->isChecked();
    allVisualGain->setEnabled(allGainEnabled);
    lowVisualGain->setEnabled(waveformsEnabled);
    midVisualGain->setEnabled(waveformsEnabled);
    highVisualGain->setEnabled(waveformsEnabled);
}

void DlgPrefWaveform::slotSetWaveformOverviewType() {
    // Apply immediately
    QVariant comboboxData = waveformOverviewComboBox->currentData();
    DEBUG_ASSERT(comboboxData.canConvert<OverviewType>());
    auto type = comboboxData.value<OverviewType>();
    m_pConfig->setValue(kOverviewTypeCfgKey, type);
    if (m_pTypeControl) {
        m_pTypeControl->forceSet(static_cast<double>(type));
    } else if (auto* pControl = ControlObject::getControl(kOverviewTypeCfgKey)) {
        pControl->forceSet(static_cast<double>(type));
    }
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotSetDefaultZoom(int index) {
    WaveformWidgetFactory::instance()->setDefaultZoom(index + 1);
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotSetZoomSynchronization(bool checked) {
    WaveformWidgetFactory::instance()->setZoomSync(checked);
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotSetVisualGainAll(double gain) {
    WaveformWidgetFactory::instance()->setVisualGain(BandIndex::AllBand, gain);
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotSetVisualGainLow(double gain) {
    WaveformWidgetFactory::instance()->setVisualGain(BandIndex::Low, gain);
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotSetVisualGainMid(double gain) {
    WaveformWidgetFactory::instance()->setVisualGain(BandIndex::Mid, gain);
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotSetVisualGainHigh(double gain) {
    WaveformWidgetFactory::instance()->setVisualGain(BandIndex::High, gain);
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotSetOverviewScaling() {
    WaveformWidgetFactory::instance()->setOverviewNormalized(
            overview_scale_normalize->isChecked());
    updateWaveformGainEnabled();
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotSetOverviewMinuteMarkers(bool draw) {
    m_pConfig->setValue(ConfigKey(kWaveformGroup,
                                QStringLiteral("draw_overview_minute_markers")),
            draw);
    if (m_pOverviewMinuteMarkersControl) {
        m_pOverviewMinuteMarkersControl->forceSet(draw);
    } else if (auto* pControl = ControlObject::getControl(
                       ConfigKey(kWaveformGroup, QStringLiteral("draw_overview_minute_markers")))) {
        pControl->forceSet(draw);
    }
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotSetOverviewStereoMode(bool stereo) {
    m_pConfig->setValue(ConfigKey(kWaveformGroup, QStringLiteral("overview_stereo_mode")), stereo);
    if (m_pOverviewStereoControl) {
        m_pOverviewStereoControl->forceSet(stereo);
    } else if (auto* pControl = ControlObject::getControl(
                       ConfigKey(kWaveformGroup, QStringLiteral("overview_stereo_mode")))) {
        pControl->forceSet(stereo);
    }
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotWaveformMeasured(float frameRate, int droppedFrames) {
    QString text = QString::number(static_cast<double>(frameRate), 'f', 2);
    if (droppedFrames < 0) {
        text += " : " + tr("dropped frames unavailable");
    } else {
        text += " : " + tr("dropped frames") + " " + QString::number(droppedFrames);
    }
    frameRateAverage->setText(text);
}

void DlgPrefWaveform::slotClearCachedWaveforms() {
    AnalysisDao analysisDao(m_pConfig);
    QSqlDatabase dbConnection = mixxx::DbConnectionPooled(m_pLibrary->dbConnectionPool());
    analysisDao.deleteAnalysesByType(dbConnection, AnalysisDao::TYPE_WAVEFORM);
    analysisDao.deleteAnalysesByType(dbConnection, AnalysisDao::TYPE_WAVESUMMARY);
    calculateCachedWaveformDiskUsage();
}

void DlgPrefWaveform::slotSetBeatGridAlpha(int alpha) {
    m_pConfig->setValue(ConfigKey(kWaveformGroup, QStringLiteral("beatGridAlpha")), alpha);
    WaveformWidgetFactory::instance()->setDisplayBeatGridAlpha(alpha);
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotSetPlayMarkerPosition(int position) {
    // QSlider works with integer values, so divide the percentage given by the
    // slider value by 100 to get a fraction of the waveform width.
    WaveformWidgetFactory::instance()->setPlayMarkerPosition(position / 100.0);
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotSetShowMemoryCueMarkers(bool checked) {
    WaveformWidgetFactory::instance()->setShowMemoryCueMarkers(checked);
    updateEnableMemoryCueMarkers();
}

void DlgPrefWaveform::slotSetMemoryCueMarkerView(int index) {
    switch (index) {
    case static_cast<int>(WaveformMark::MemoryCueMarkerView::ScrollingWaveformsOnly):
        WaveformWidgetFactory::instance()->setMemoryCueMarkerView(
                WaveformMark::MemoryCueMarkerView::ScrollingWaveformsOnly);
        break;
    case static_cast<int>(WaveformMark::MemoryCueMarkerView::OverviewOnly):
        WaveformWidgetFactory::instance()->setMemoryCueMarkerView(
                WaveformMark::MemoryCueMarkerView::OverviewOnly);
        break;
    default:
        WaveformWidgetFactory::instance()->setMemoryCueMarkerView(
                WaveformMark::MemoryCueMarkerView::Both);
        break;
    }
}

void DlgPrefWaveform::slotSetMemoryCueMarkerAlign(int index) {
    switch (index) {
    case 0:
        WaveformWidgetFactory::instance()->setMemoryCueMarkerAlign(Qt::AlignTop);
        break;
    case 2:
        WaveformWidgetFactory::instance()->setMemoryCueMarkerAlign(
                Qt::AlignTop | Qt::AlignBottom);
        break;
    default:
        WaveformWidgetFactory::instance()->setMemoryCueMarkerAlign(Qt::AlignBottom);
        break;
    }
}

void DlgPrefWaveform::slotSetMemoryCueMarkerLineWidth(int width) {
    WaveformWidgetFactory::instance()->setMemoryCueMarkerLineWidth(width);
}

void DlgPrefWaveform::updateEnableMemoryCueMarkers() {
    const bool enabled = showMemoryCueMarkersCheckBox->isChecked();
    memoryCueMarkerViewComboBox->setEnabled(enabled);
    memoryCueMarkerAlignLabel->setEnabled(enabled);
    memoryCueMarkerAlignComboBox->setEnabled(enabled);
    memoryCueMarkerLineWidthLabel->setEnabled(enabled);
    memoryCueMarkerLineWidthSpinBox->setEnabled(enabled);
    memoryCueMarkerSizeLabel->setEnabled(enabled);
    memoryCueMarkerOpacityLabel->setEnabled(enabled);
    memoryCueMarkerOpacitySpinBox->setEnabled(enabled);
    memoryCueMarkerSizeSpinBox->setEnabled(enabled);
    memoryCueMarkerCustomOutlineCheckBox->setEnabled(enabled);
    memoryCueMarkerOutlineColorButton->setEnabled(
            enabled && memoryCueMarkerCustomOutlineCheckBox->isChecked());
    memoryCueMarkerSingleColorCheckBox->setEnabled(enabled);
    memoryCueMarkerColorButton->setEnabled(
            enabled && memoryCueMarkerSingleColorCheckBox->isChecked());
    memoryCueMarkerShowNamesCheckBox->setEnabled(enabled);
    const bool namesEnabled = enabled && memoryCueMarkerShowNamesCheckBox->isChecked();
    memoryCueNameAlignLabel->setEnabled(namesEnabled);
    memoryCueNameAlignComboBox->setEnabled(namesEnabled);
    memoryCueNameBackgroundLabel->setEnabled(namesEnabled);
    memoryCueNameBackgroundComboBox->setEnabled(namesEnabled);
    memoryCueNameBackgroundColorButton->setEnabled(namesEnabled &&
            memoryCueNameBackgroundComboBox->currentIndex() ==
                    static_cast<int>(WaveformMark::MemoryCueNameBackground::CustomColor));
    memoryCueNameCustomTextColorCheckBox->setEnabled(namesEnabled);
    memoryCueNameTextColorButton->setEnabled(
            namesEnabled && memoryCueNameCustomTextColorCheckBox->isChecked());
}

void DlgPrefWaveform::slotSetMemoryCueMarkerShowNames(bool checked) {
    WaveformWidgetFactory::instance()->setMemoryCueMarkerShowNames(checked);
    updateEnableMemoryCueMarkers();
}

void DlgPrefWaveform::slotSetMemoryCueNameAlign(int index) {
    WaveformWidgetFactory::instance()->setMemoryCueNameAlign(
            index == 1 ? Qt::AlignBottom : Qt::AlignTop);
}

void DlgPrefWaveform::slotSetMemoryCueNameBackground(int index) {
    switch (index) {
    case static_cast<int>(WaveformMark::MemoryCueNameBackground::Off):
        WaveformWidgetFactory::instance()->setMemoryCueNameBackground(
                WaveformMark::MemoryCueNameBackground::Off);
        break;
    case static_cast<int>(WaveformMark::MemoryCueNameBackground::CustomColor):
        WaveformWidgetFactory::instance()->setMemoryCueNameBackground(
                WaveformMark::MemoryCueNameBackground::CustomColor);
        break;
    default:
        WaveformWidgetFactory::instance()->setMemoryCueNameBackground(
                WaveformMark::MemoryCueNameBackground::CueColor);
        break;
    }
    updateEnableMemoryCueMarkers();
}

void DlgPrefWaveform::slotChooseMemoryCueNameBackgroundColor() {
    const QColor oldColor =
            WaveformWidgetFactory::instance()->getMemoryCueNameBackgroundCustomColor();
    const QColor newColor = QColorDialog::getColor(oldColor,
            this,
            tr("Memory cue name background color"),
            QColorDialog::ShowAlphaChannel);
    if (!newColor.isValid()) {
        return; // dialog canceled
    }
    WaveformWidgetFactory::instance()->setMemoryCueNameBackgroundCustomColor(newColor);
    updateMemoryCueNameBackgroundColorButton();
}

void DlgPrefWaveform::updateMemoryCueNameBackgroundColorButton() {
    const QColor color =
            WaveformWidgetFactory::instance()->getMemoryCueNameBackgroundCustomColor();
    memoryCueNameBackgroundColorButton->setStyleSheet(
            QStringLiteral("QPushButton { background-color: rgba(%1, %2, %3, %4); }")
                    .arg(color.red())
                    .arg(color.green())
                    .arg(color.blue())
                    .arg(color.alpha()));
    memoryCueNameBackgroundColorButton->setToolTip(
            tr("Custom background color of the memory cue names (%1, %2 % opaque). "
               "Click to change.")
                    .arg(color.name())
                    .arg(qRound(color.alphaF() * 100)));
}

void DlgPrefWaveform::slotSetMemoryCueNameCustomTextColor(bool checked) {
    WaveformWidgetFactory::instance()->setMemoryCueNameCustomTextColor(checked);
    updateEnableMemoryCueMarkers();
}

void DlgPrefWaveform::slotChooseMemoryCueNameTextColor() {
    const QColor oldColor = WaveformWidgetFactory::instance()->getMemoryCueNameTextColor();
    const QColor newColor = QColorDialog::getColor(
            oldColor, this, tr("Memory cue name text color"));
    if (!newColor.isValid()) {
        return; // dialog canceled
    }
    WaveformWidgetFactory::instance()->setMemoryCueNameTextColor(newColor);
    updateMemoryCueNameTextColorButton();
}

void DlgPrefWaveform::updateMemoryCueNameTextColorButton() {
    const QColor color = WaveformWidgetFactory::instance()->getMemoryCueNameTextColor();
    memoryCueNameTextColorButton->setStyleSheet(
            QStringLiteral("QPushButton { background-color: %1; }").arg(color.name()));
    memoryCueNameTextColorButton->setToolTip(
            tr("Text color of the memory cue names (%1). Click to change.")
                    .arg(color.name()));
}

void DlgPrefWaveform::slotSetMemoryCueMarkerSize(int size) {
    WaveformWidgetFactory::instance()->setMemoryCueMarkerSize(size);
}

void DlgPrefWaveform::slotSetMemoryCueMarkerOpacity(int percent) {
    WaveformWidgetFactory::instance()->setMemoryCueMarkerOpacity(percent);
}

void DlgPrefWaveform::slotSetMemoryCueMarkerCustomOutline(bool checked) {
    WaveformWidgetFactory::instance()->setMemoryCueMarkerCustomOutline(checked);
    updateEnableMemoryCueMarkers();
}

void DlgPrefWaveform::slotChooseMemoryCueMarkerOutlineColor() {
    const QColor oldColor = WaveformWidgetFactory::instance()->getMemoryCueMarkerOutlineColor();
    const QColor newColor = QColorDialog::getColor(
            oldColor, this, tr("Memory cue marker outline color"));
    if (!newColor.isValid()) {
        return; // dialog canceled
    }
    WaveformWidgetFactory::instance()->setMemoryCueMarkerOutlineColor(newColor);
    updateMemoryCueMarkerOutlineColorButton();
}

void DlgPrefWaveform::slotSetMemoryCueMarkerSingleColor(bool checked) {
    WaveformWidgetFactory::instance()->setMemoryCueMarkerSingleColor(checked);
    updateEnableMemoryCueMarkers();
}

void DlgPrefWaveform::slotChooseMemoryCueMarkerColor() {
    const QColor oldColor = WaveformWidgetFactory::instance()->getMemoryCueMarkerColor();
    const QColor newColor = QColorDialog::getColor(
            oldColor, this, tr("Memory cue marker color"));
    if (!newColor.isValid()) {
        return; // dialog canceled
    }
    WaveformWidgetFactory::instance()->setMemoryCueMarkerColor(newColor);
    updateMemoryCueMarkerColorButton();
}

void DlgPrefWaveform::updateMemoryCueMarkerColorButton() {
    const QColor color = WaveformWidgetFactory::instance()->getMemoryCueMarkerColor();
    memoryCueMarkerColorButton->setStyleSheet(
            QStringLiteral("QPushButton { background-color: %1; }").arg(color.name()));
    memoryCueMarkerColorButton->setToolTip(
            tr("Color of all memory cue markers (%1). Click to change.").arg(color.name()));
}

void DlgPrefWaveform::updateMemoryCueMarkerOutlineColorButton() {
    const QColor color = WaveformWidgetFactory::instance()->getMemoryCueMarkerOutlineColor();
    memoryCueMarkerOutlineColorButton->setStyleSheet(
            QStringLiteral("QPushButton { background-color: %1; }").arg(color.name()));
    memoryCueMarkerOutlineColorButton->setToolTip(
            tr("Outline color of the memory cue triangles (%1). Click to change.")
                    .arg(color.name()));
}

void DlgPrefWaveform::slotSetUntilMarkShowBeats(bool checked) {
    WaveformWidgetFactory::instance()->setUntilMarkShowBeats(checked);
    updateEnableUntilMark();
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotSetUntilMarkShowTime(bool checked) {
    WaveformWidgetFactory::instance()->setUntilMarkShowTime(checked);
    updateEnableUntilMark();
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotSetUntilMarkAlign(int index) {
    WaveformWidgetFactory::instance()->setUntilMarkAlign(
            WaveformWidgetFactory::toUntilMarkAlign(index));
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotSetUntilMarkTextPointSize(int value) {
    WaveformWidgetFactory::instance()->setUntilMarkTextPointSize(value);
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotSetUntilMarkTextHeightLimit(int index) {
    WaveformWidgetFactory::instance()->setUntilMarkTextHeightLimit(
            WaveformWidgetFactory::toUntilMarkTextHeightLimit(index));
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotStemOpacity(double value) {
    WaveformWidgetFactory::instance()->setStemOpacity(static_cast<float>(value));
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotStemReorderOnChange(bool value) {
    WaveformWidgetFactory::instance()->setStemReorderOnChange(value);
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotStemOutlineOpacity(double value) {
    WaveformWidgetFactory::instance()->setStemOutlineOpacity(static_cast<float>(value));
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::slotStemDisplayMode(int index) {
    WaveformWidgetFactory::instance()->setStemSplitTracks(index == 1);
    updateStemOptionsEnabled();
    notifyQmlWaveformSettingsChanged();
}

void DlgPrefWaveform::calculateCachedWaveformDiskUsage() {
    AnalysisDao analysisDao(m_pConfig);
    QSqlDatabase dbConnection = mixxx::DbConnectionPooled(m_pLibrary->dbConnectionPool());
    size_t numBytes = analysisDao.getDiskUsageInBytes(dbConnection, AnalysisDao::TYPE_WAVEFORM) +
            analysisDao.getDiskUsageInBytes(dbConnection, AnalysisDao::TYPE_WAVESUMMARY);

    QString sizeText = QLocale().formattedDataSize(numBytes, 1, QLocale::DataSizeSIFormat);
    waveformDiskUsage->setText(
            tr("Cached waveforms occupy %1 on disk.").arg(sizeText));
}
