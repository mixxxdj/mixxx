#include "preferences/dialog/dlgprefwaveform.h"

#include "library/dao/analysisdao.h"
#include "library/library.h"
#include "mixxx.h"
#include "moc_dlgprefwaveform.cpp"
#include "preferences/waveformsettings.h"
#include "util/db/dbconnectionpooled.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveformwidgetfactory.h"

DlgPrefWaveform::DlgPrefWaveform(
        QWidget* pParent,
        MixxxMainWindow* pMixxx,
        UserSettingsPointer pConfig,
        std::shared_ptr<Library> pLibrary)
        : DlgPreferencePage(pParent),
          m_pConfig(pConfig),
          m_pLibrary(pLibrary),
          m_pMixxx(pMixxx) {
    setupUi(this);

    // Waveform overview init
    waveformOverviewComboBox->addItem(tr("Filtered")); // "0"
    waveformOverviewComboBox->addItem(tr("HSV")); // "1"
    waveformOverviewComboBox->addItem(tr("RGB")); // "2"

    // Populate waveform options.
    WaveformWidgetFactory* factory = WaveformWidgetFactory::instance();
    QVector<WaveformWidgetAbstractHandle> handles = factory->getAvailableTypes();
    for (int i = 0; i < handles.size(); ++i) {
        waveformTypeComboBox->addItem(handles[i].getDisplayName(),
                                      handles[i].getType());
    }

    // Populate zoom options.
    for (int i = static_cast<int>(WaveformWidgetRenderer::s_waveformMinZoom);
            i <= static_cast<int>(WaveformWidgetRenderer::s_waveformMaxZoom);
            i++) {
        defaultZoomComboBox->addItem(QString::number(100 / static_cast<double>(i), 'f', 1) + " %");
    }

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

    connect(waveformTypeComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefWaveform::slotSetWaveformType);
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
    connect(normalizeOverviewCheckBox,
            &QCheckBox::toggled,
            this,
            &DlgPrefWaveform::slotSetNormalizeOverview);
    connect(factory,
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
}

DlgPrefWaveform::~DlgPrefWaveform() {
}

void DlgPrefWaveform::slotUpdate() {
    WaveformWidgetFactory* factory = WaveformWidgetFactory::instance();

    if (factory->isOpenGlAvailable() || factory->isOpenGlesAvailable()) {
        openGlStatusIcon->setText(factory->getOpenGLVersion());
    } else {
        openGlStatusIcon->setText(tr("OpenGL not available") + ": " + factory->getOpenGLVersion());
    }

    WaveformWidgetType::Type currentType = factory->getType();
    int currentIndex = waveformTypeComboBox->findData(currentType);
    if (currentIndex != -1 && waveformTypeComboBox->currentIndex() != currentIndex) {
        waveformTypeComboBox->setCurrentIndex(currentIndex);
    }

    frameRateSpinBox->setValue(factory->getFrameRate());
    frameRateSlider->setValue(factory->getFrameRate());
    endOfTrackWarningTimeSpinBox->setValue(factory->getEndOfTrackWarningTime());
    endOfTrackWarningTimeSlider->setValue(factory->getEndOfTrackWarningTime());
    synchronizeZoomCheckBox->setChecked(factory->isZoomSync());
    allVisualGain->setValue(factory->getVisualGain(WaveformWidgetFactory::All));
    lowVisualGain->setValue(factory->getVisualGain(WaveformWidgetFactory::Low));
    midVisualGain->setValue(factory->getVisualGain(WaveformWidgetFactory::Mid));
    highVisualGain->setValue(factory->getVisualGain(WaveformWidgetFactory::High));
    normalizeOverviewCheckBox->setChecked(factory->isOverviewNormalized());
    // Round zoom to int to get a default zoom index.
    defaultZoomComboBox->setCurrentIndex(static_cast<int>(factory->getDefaultZoom()) - 1);
    playMarkerPositionSlider->setValue(static_cast<int>(factory->getPlayMarkerPosition() * 100));
    beatGridAlphaSpinBox->setValue(factory->getBeatGridAlpha());
    beatGridAlphaSlider->setValue(factory->getBeatGridAlpha());

    // By default we set RGB woverview = "2"
    int overviewType = m_pConfig->getValue(
            ConfigKey("[Waveform]","WaveformOverviewType"), 2);
    if (overviewType != waveformOverviewComboBox->currentIndex()) {
        waveformOverviewComboBox->setCurrentIndex(overviewType);
    }

    WaveformSettings waveformSettings(m_pConfig);
    enableWaveformCaching->setChecked(waveformSettings.waveformCachingEnabled());
    enableWaveformGenerationWithAnalysis->setChecked(
        waveformSettings.waveformGenerationWithAnalysisEnabled());
    calculateCachedWaveformDiskUsage();
}

void DlgPrefWaveform::slotApply() {
    ConfigValue overviewtype = ConfigValue(waveformOverviewComboBox->currentIndex());
    if (overviewtype != m_pConfig->get(ConfigKey("[Waveform]", "WaveformOverviewType"))) {
        m_pConfig->set(ConfigKey("[Waveform]", "WaveformOverviewType"), overviewtype);
    }
    WaveformSettings waveformSettings(m_pConfig);
    waveformSettings.setWaveformCachingEnabled(enableWaveformCaching->isChecked());
    waveformSettings.setWaveformGenerationWithAnalysisEnabled(
        enableWaveformGenerationWithAnalysis->isChecked());
}

void DlgPrefWaveform::slotResetToDefaults() {
    WaveformWidgetFactory* factory = WaveformWidgetFactory::instance();

    // Get the default we ought to use based on whether the user has OpenGL or
    // not.
    WaveformWidgetType::Type defaultType = factory->autoChooseWidgetType();
    int defaultIndex = waveformTypeComboBox->findData(defaultType);
    if (defaultIndex != -1 && waveformTypeComboBox->currentIndex() != defaultIndex) {
        waveformTypeComboBox->setCurrentIndex(defaultIndex);
    }

    allVisualGain->setValue(1.0);
    lowVisualGain->setValue(1.0);
    midVisualGain->setValue(1.0);
    highVisualGain->setValue(1.0);

    // Default zoom level is 3 in WaveformWidgetFactory.
    defaultZoomComboBox->setCurrentIndex(3 + 1);

    synchronizeZoomCheckBox->setChecked(true);

    // RGB overview.
    waveformOverviewComboBox->setCurrentIndex(2);

    // Don't normalize overview.
    normalizeOverviewCheckBox->setChecked(false);

    // 30FPS is the default
    frameRateSlider->setValue(30);
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
    WaveformWidgetFactory::instance()->setWidgetTypeFromHandle(index);
}

void DlgPrefWaveform::slotSetWaveformOverviewType(int index) {
    m_pConfig->set(ConfigKey("[Waveform]","WaveformOverviewType"), ConfigValue(index));
    m_pMixxx->rebootMixxxView();
}

void DlgPrefWaveform::slotSetDefaultZoom(int index) {
    WaveformWidgetFactory::instance()->setDefaultZoom(index + 1);
}

void DlgPrefWaveform::slotSetZoomSynchronization(bool checked) {
    WaveformWidgetFactory::instance()->setZoomSync(checked);
}

void DlgPrefWaveform::slotSetVisualGainAll(double gain) {
    WaveformWidgetFactory::instance()->setVisualGain(WaveformWidgetFactory::All,gain);
}

void DlgPrefWaveform::slotSetVisualGainLow(double gain) {
    WaveformWidgetFactory::instance()->setVisualGain(WaveformWidgetFactory::Low,gain);
}

void DlgPrefWaveform::slotSetVisualGainMid(double gain) {
    WaveformWidgetFactory::instance()->setVisualGain(WaveformWidgetFactory::Mid,gain);
}

void DlgPrefWaveform::slotSetVisualGainHigh(double gain) {
    WaveformWidgetFactory::instance()->setVisualGain(WaveformWidgetFactory::High,gain);
}

void DlgPrefWaveform::slotSetNormalizeOverview(bool normalize) {
    WaveformWidgetFactory::instance()->setOverviewNormalized(normalize);
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
    m_pConfig->setValue(ConfigKey("[Waveform]", "beatGridAlpha"), alpha);
    WaveformWidgetFactory::instance()->setDisplayBeatGridAlpha(alpha);
}

void DlgPrefWaveform::slotSetPlayMarkerPosition(int position) {
    // QSlider works with integer values, so divide the percentage given by the
    // slider value by 100 to get a fraction of the waveform width.
    WaveformWidgetFactory::instance()->setPlayMarkerPosition(position / 100.0);
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
