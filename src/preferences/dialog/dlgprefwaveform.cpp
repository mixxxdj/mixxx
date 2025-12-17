#include "preferences/dialog/dlgprefwaveform.h"

#include <QMetaEnum>

#include "control/controlpushbutton.h"
#include "library/dao/analysisdao.h"
#include "library/library.h"
#include "moc_dlgprefwaveform.cpp"
#include "preferences/waveformsettings.h"
#include "util/db/dbconnectionpooled.h"
#include "waveform/overviewtype.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveformwidgetfactory.h"

namespace {
const QString kWaveformGroup(QStringLiteral("[Waveform]"));
const ConfigKey kOverviewTypeCfgKey(kWaveformGroup,
        QStringLiteral("WaveformOverviewType"));
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

    // Waveform overview init
    waveformOverviewComboBox->addItem(
            tr("Filtered"), QVariant::fromValue(OverviewType::Filtered));
    waveformOverviewComboBox->addItem(tr("HSV"), QVariant::fromValue(OverviewType::HSV));
    waveformOverviewComboBox->addItem(tr("RGB"), QVariant::fromValue(OverviewType::RGB));
    m_pTypeControl = std::make_unique<ControlPushButton>(kOverviewTypeCfgKey);
    m_pTypeControl->setStates(QMetaEnum::fromType<OverviewType>().keyCount());
    m_pTypeControl->setReadOnly();
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
    m_pTypeControl->forceSet(cfgTypeIndex);

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

    m_pOverviewMinuteMarkersControl = std::make_unique<ControlObject>(
            ConfigKey(kWaveformGroup, QStringLiteral("draw_overview_minute_markers")));
    m_pOverviewMinuteMarkersControl->setReadOnly();

    // Populate untilMark options
    untilMarkAlignComboBox->addItem(tr("Top"));
    untilMarkAlignComboBox->addItem(tr("Center"));
    untilMarkAlignComboBox->addItem(tr("Bottom"));

    //: options for "Text height limit"
    untilMarkTextHeightLimitComboBox->addItem(tr("1/3 of waveform viewer"));
    untilMarkTextHeightLimitComboBox->addItem(tr("Entire waveform viewer"));

    // Adopt tr string from first GLSL hint
    requiresGLSLLabel2->setText(requiresGLSLLabel->text());

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

    setScrollSafeGuardForAllInputWidgets(this);
}

void DlgPrefWaveform::slotSetWaveformOptions(
        allshader::WaveformRendererSignalBase::Option option, bool enabled) {
    auto* pFactory = WaveformWidgetFactory::instance();
    pFactory->setWaveformOption(option, enabled);
    auto type = static_cast<WaveformWidgetType::Type>(
            waveformTypeComboBox->currentData().toInt());
    pFactory->setWidgetTypeFromHandle(
            pFactory->findHandleIndexFromType(type), true);
}

void DlgPrefWaveform::slotUpdate() {
    auto* pFactory = WaveformWidgetFactory::instance();
    if (pFactory->isOpenGlAvailable() || pFactory->isOpenGlesAvailable()) {
        openGlStatusData->setText(pFactory->getOpenGLVersion());
        useAccelerationCheckBox->setEnabled(true);
        bool isAccelerationEnabled =
                pFactory->getBackendFromConfig() != WaveformWidgetBackend::None;
        useAccelerationCheckBox->setChecked(isAccelerationEnabled);
    } else {
        openGlStatusData->setText(tr("OpenGL not available") + ": " + pFactory->getOpenGLVersion());
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

    bool drawOverviewMinuteMarkers = m_pConfig->getValue(
            ConfigKey(kWaveformGroup, QStringLiteral("draw_overview_minute_markers")), true);
    overviewMinuteMarkersCheckBox->setChecked(drawOverviewMinuteMarkers);
    m_pOverviewMinuteMarkersControl->forceSet(drawOverviewMinuteMarkers);

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
    beatGridAlphaSpinBox->setValue(90);

    // 50 (center) is default
    playMarkerPositionSlider->setValue(50);
}

void DlgPrefWaveform::slotSetFrameRate(int frameRate) {
    WaveformWidgetFactory::instance()->setFrameRate(frameRate);
}

void DlgPrefWaveform::slotSetWaveformEndRender(int endTime) {
    WaveformWidgetFactory::instance()->setEndOfTrackWarningTime(endTime);
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
}

void DlgPrefWaveform::slotSetWaveformAcceleration(bool checked) {
    auto* pFactory = WaveformWidgetFactory::instance();
    pFactory->setAcceleration(checked);
    auto type = static_cast<WaveformWidgetType::Type>(waveformTypeComboBox->currentData().toInt());
    pFactory->setWidgetTypeFromHandle(pFactory->findHandleIndexFromType(type), true);
    updateWaveformTypeOptions(true);
    updateEnableUntilMark();
    updateStemOptionsEnabled();
}

void DlgPrefWaveform::updateWaveformAcceleration(WaveformWidgetType::Type type) {
    auto* pFactory = WaveformWidgetFactory::instance();
    auto backend = pFactory->getBackendFromConfig();

    bool supportAcceleration = pFactory->widgetTypeSupportsAcceleration(type);
    bool supportSoftware = pFactory->widgetTypeSupportsSoftware(type);

    useAccelerationCheckBox->blockSignals(true);

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
    auto backend = pFactory->getBackendFromConfig();
    auto currentOptions = pFactory->getWaveformOptions();
    auto supportedOptions = pFactory->getWaveformOptionsSupportedByType(type, backend);

    splitLeftRightCheckBox->setEnabled(useWaveform &&
            (supportedOptions & allshader::WaveformRendererSignalBase::Option::SplitStereoSignal));
    highDetailCheckBox->setEnabled(useWaveform &&
            (supportedOptions & allshader::WaveformRendererSignalBase::Option::HighDetail));
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
    stemOpacityMainLabel->setEnabled(stemsSupported && enabled);
    stemOpacityOutlineLabel->setEnabled(stemsSupported && enabled);
    stemReorderLayerOnChangedCheckBox->setEnabled(stemsSupported && enabled);
    stemOpacitySpinBox->setEnabled(stemsSupported && enabled);
    stemOutlineOpacitySpinBox->setEnabled(stemsSupported && enabled);
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
    m_pTypeControl->forceSet(static_cast<double>(type));
}

void DlgPrefWaveform::slotSetDefaultZoom(int index) {
    WaveformWidgetFactory::instance()->setDefaultZoom(index + 1);
}

void DlgPrefWaveform::slotSetZoomSynchronization(bool checked) {
    WaveformWidgetFactory::instance()->setZoomSync(checked);
}

void DlgPrefWaveform::slotSetVisualGainAll(double gain) {
    WaveformWidgetFactory::instance()->setVisualGain(BandIndex::AllBand, gain);
}

void DlgPrefWaveform::slotSetVisualGainLow(double gain) {
    WaveformWidgetFactory::instance()->setVisualGain(BandIndex::Low, gain);
}

void DlgPrefWaveform::slotSetVisualGainMid(double gain) {
    WaveformWidgetFactory::instance()->setVisualGain(BandIndex::Mid, gain);
}

void DlgPrefWaveform::slotSetVisualGainHigh(double gain) {
    WaveformWidgetFactory::instance()->setVisualGain(BandIndex::High, gain);
}

void DlgPrefWaveform::slotSetOverviewScaling() {
    WaveformWidgetFactory::instance()->setOverviewNormalized(
            overview_scale_normalize->isChecked());
    updateWaveformGainEnabled();
}

void DlgPrefWaveform::slotSetOverviewMinuteMarkers(bool draw) {
    m_pConfig->setValue(ConfigKey(kWaveformGroup,
                                QStringLiteral("draw_overview_minute_markers")),
            draw);
    m_pOverviewMinuteMarkersControl->forceSet(draw);
}

void DlgPrefWaveform::slotWaveformMeasured(float frameRate, int droppedFrames) {
    frameRateAverage->setText(
            QString::number((double)frameRate, 'f', 2) + " : " +
            tr("dropped frames") + " " + QString::number(droppedFrames));
}

void DlgPrefWaveform::slotClearCachedWaveforms() {
    AnalysisDao analysisDao(m_pConfig);
    QSqlDatabase dbConnection = mixxx::DbConnectionPooled(m_pLibrary->dbConnectionPool());
    analysisDao.deleteAnalysesByType(dbConnection, AnalysisDao::TYPE_WAVEFORM);
    analysisDao.deleteAnalysesByType(dbConnection, AnalysisDao::TYPE_WAVESUMMARY);
    calculateCachedWaveformDiskUsage();
}

void DlgPrefWaveform::slotSetBeatGridAlpha(int alpha) {
    WaveformWidgetFactory::instance()->setDisplayBeatGridAlpha(alpha);
}

void DlgPrefWaveform::slotSetPlayMarkerPosition(int position) {
    // QSlider works with integer values, so divide the percentage given by the
    // slider value by 100 to get a fraction of the waveform width.
    WaveformWidgetFactory::instance()->setPlayMarkerPosition(position / 100.0);
}

void DlgPrefWaveform::slotSetUntilMarkShowBeats(bool checked) {
    WaveformWidgetFactory::instance()->setUntilMarkShowBeats(checked);
    updateEnableUntilMark();
}

void DlgPrefWaveform::slotSetUntilMarkShowTime(bool checked) {
    WaveformWidgetFactory::instance()->setUntilMarkShowTime(checked);
    updateEnableUntilMark();
}

void DlgPrefWaveform::slotSetUntilMarkAlign(int index) {
    WaveformWidgetFactory::instance()->setUntilMarkAlign(
            WaveformWidgetFactory::toUntilMarkAlign(index));
}

void DlgPrefWaveform::slotSetUntilMarkTextPointSize(int value) {
    WaveformWidgetFactory::instance()->setUntilMarkTextPointSize(value);
}

void DlgPrefWaveform::slotSetUntilMarkTextHeightLimit(int index) {
    WaveformWidgetFactory::instance()->setUntilMarkTextHeightLimit(
            WaveformWidgetFactory::toUntilMarkTextHeightLimit(index));
}

void DlgPrefWaveform::slotStemOpacity(float value) {
    WaveformWidgetFactory::instance()->setStemOpacity(value);
}

void DlgPrefWaveform::slotStemReorderOnChange(bool value) {
    WaveformWidgetFactory::instance()->setStemReorderOnChange(value);
}

void DlgPrefWaveform::slotStemOutlineOpacity(float value) {
    WaveformWidgetFactory::instance()->setStemOutlineOpacity(value);
}

void DlgPrefWaveform::calculateCachedWaveformDiskUsage() {
    AnalysisDao analysisDao(m_pConfig);
    QSqlDatabase dbConnection = mixxx::DbConnectionPooled(m_pLibrary->dbConnectionPool());
    size_t numBytes = analysisDao.getDiskUsageInBytes(dbConnection, AnalysisDao::TYPE_WAVEFORM) +
            analysisDao.getDiskUsageInBytes(dbConnection, AnalysisDao::TYPE_WAVESUMMARY);

    // Display total cached waveform size in mebibytes with 2 decimals.
    QString sizeMebibytes = QString::number(
            numBytes / (1024.0 * 1024.0), 'f', 2);

    waveformDiskUsage->setText(
            tr("Cached waveforms occupy %1 MiB on disk.").arg(sizeMebibytes));
}
