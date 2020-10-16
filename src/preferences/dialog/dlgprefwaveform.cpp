#include "preferences/dialog/dlgprefwaveform.h"

#include "mixxx.h"
#include "library/library.h"
#include "library/dao/analysisdao.h"
#include "preferences/waveformsettings.h"
#include "waveform/waveformwidgetfactory.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "util/db/dbconnectionpooled.h"

DlgPrefWaveform::DlgPrefWaveform(QWidget* pParent, MixxxMainWindow* pMixxx,
                                 UserSettingsPointer pConfig, Library* pLibrary)
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
            SIGNAL(valueChanged(int)),
            this,
            SLOT(slotSetFrameRate(int)));
    connect(endOfTrackWarningTimeSpinBox,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(slotSetWaveformEndRender(int)));
    connect(beatGridAlphaSpinBox,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(slotSetBeatGridAlpha(int)));
    connect(frameRateSlider,
            SIGNAL(valueChanged(int)),
            frameRateSpinBox,
            SLOT(setValue(int)));
    connect(frameRateSpinBox,
            SIGNAL(valueChanged(int)),
            frameRateSlider,
            SLOT(setValue(int)));
    connect(endOfTrackWarningTimeSlider,
            SIGNAL(valueChanged(int)),
            endOfTrackWarningTimeSpinBox,
            SLOT(setValue(int)));
    connect(endOfTrackWarningTimeSpinBox,
            SIGNAL(valueChanged(int)),
            endOfTrackWarningTimeSlider,
            SLOT(setValue(int)));
    connect(beatGridAlphaSlider,
            SIGNAL(valueChanged(int)),
            beatGridAlphaSpinBox,
            SLOT(setValue(int)));
    connect(beatGridAlphaSpinBox,
            SIGNAL(valueChanged(int)),
            beatGridAlphaSlider,
            SLOT(setValue(int)));

    connect(waveformTypeComboBox,
            SIGNAL(currentIndexChanged(int)),
            this,
            SLOT(slotSetWaveformType(int)));
    connect(defaultZoomComboBox,
            SIGNAL(currentIndexChanged(int)),
            this,
            SLOT(slotSetDefaultZoom(int)));
    connect(synchronizeZoomCheckBox,
            SIGNAL(clicked(bool)),
            this,
            SLOT(slotSetZoomSynchronization(bool)));
    connect(allVisualGain,
            SIGNAL(valueChanged(double)),
            this,
            SLOT(slotSetVisualGainAll(double)));
    connect(lowVisualGain,
            SIGNAL(valueChanged(double)),
            this,
            SLOT(slotSetVisualGainLow(double)));
    connect(midVisualGain,
            SIGNAL(valueChanged(double)),
            this,
            SLOT(slotSetVisualGainMid(double)));
    connect(highVisualGain,
            SIGNAL(valueChanged(double)),
            this,
            SLOT(slotSetVisualGainHigh(double)));
    connect(normalizeOverviewCheckBox,
            SIGNAL(toggled(bool)),
            this,
            SLOT(slotSetNormalizeOverview(bool)));
    connect(factory,
            SIGNAL(waveformMeasured(float, int)),
            this,
            SLOT(slotWaveformMeasured(float, int)));
    connect(waveformOverviewComboBox,
            SIGNAL(currentIndexChanged(int)),
            this,
            SLOT(slotSetWaveformOverviewType(int)));
    connect(clearCachedWaveforms,
            SIGNAL(clicked()),
            this,
            SLOT(slotClearCachedWaveforms()));
    connect(playMarkerPositionSlider,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(slotSetPlayMarkerPosition(int)));
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

    // Don't synchronize zoom by default.
    synchronizeZoomCheckBox->setChecked(false);

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
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0) && defined __WINDOWS__
    // This regenerates the waveforms twice because of a bug found on Windows
    // where the first one fails.
    // The problem is that the window of the widget thinks that it is not exposed.
    // (https://doc.qt.io/qt-5/qwindow.html#exposeEvent )
    // TODO: Remove this when it has been fixed upstream.
    WaveformWidgetFactory::instance()->setWidgetTypeFromHandle(index, true);
#endif
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
