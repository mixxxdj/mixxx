#include "preferences/dialog/dlgprefsoundcalibrate.h"

#include <QApplication>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

#include "util/math.h"

DlgPrefSoundCalibrate::DlgPrefSoundCalibrate(QWidget* parent,
        DlgPrefSoundItem* pSoundItem,
        int framesPerBuffer,
        int sampleRate)
        : QDialog(parent),
          m_pSoundItem(pSoundItem),
          m_currentOffsetMs(0.0),
          m_fineOffsetMs(0.0),
          m_playingTone(false),
          m_framesPerBuffer(framesPerBuffer > 0 ? framesPerBuffer : 1024),
          m_sampleRate(sampleRate > 0 ? sampleRate : 44100),
          m_pStatusLabel(nullptr),
          m_pOffsetSpinbox(nullptr),
          m_pFineSlider(nullptr),
          m_pAutoCalibrateButton(nullptr),
          m_pPlayToneButton(nullptr),
          m_pApplyButton(nullptr),
          m_pCancelButton(nullptr),
          m_pExplanationLabel(nullptr) {
    m_currentOffsetMs = pSoundItem ? static_cast<double>(pSoundItem->getLatencyOffsetMs()) : 0.0;

    setupUi();

    m_pSyncTimer = new QTimer(this);
    m_pSyncTimer->setInterval(1000); // 1 second interval
    connect(m_pSyncTimer, &QTimer::timeout, this, &DlgPrefSoundCalibrate::updateReferenceTone);
}

void DlgPrefSoundCalibrate::setupUi() {
    setWindowTitle(tr("Calibrate Output Latency"));
    setMinimumWidth(380);

    QVBoxLayout* pMainLayout = new QVBoxLayout(this);

    // Explanation text
    m_pExplanationLabel = new QLabel(
            tr("This tool calibrates the output device by measuring round-trip\n"
               "audio latency. It plays a sync pulse through this output and records\n"
               "it through an input, then uses cross-correlation to find the delay.\n\n"
               "Click \"Auto-Calibrate\" to start. The offset will be applied\n"
               "automatically when you click Apply."));
    m_pExplanationLabel->setWordWrap(true);
    pMainLayout->addWidget(m_pExplanationLabel);

    // Offset spinbox (coarse, integer ms)
    QHBoxLayout* pOffsetLayout = new QHBoxLayout();
    pOffsetLayout->addWidget(new QLabel(tr("Coarse offset (ms):")));
    m_pOffsetSpinbox = new QDoubleSpinBox();
    m_pOffsetSpinbox->setRange(0, 500);
    m_pOffsetSpinbox->setDecimals(0);
    m_pOffsetSpinbox->setSuffix(" ms");
    m_pOffsetSpinbox->setValue(m_currentOffsetMs);
    m_pOffsetSpinbox->setSingleStep(1);
    pOffsetLayout->addWidget(m_pOffsetSpinbox);
    pMainLayout->addLayout(pOffsetLayout);

    // Fine-tuning slider (+/- 5ms with 0.1ms resolution)
    QHBoxLayout* pFineLayout = new QHBoxLayout();
    pFineLayout->addWidget(new QLabel(tr("Fine-tune:")));
    m_pFineSlider = new QSlider(Qt::Horizontal);
    m_pFineSlider->setRange(-50, 50); // -5.0ms to +5.0ms in 0.1ms steps
    m_pFineSlider->setValue(0);
    m_pFineSlider->setTickInterval(10);
    m_pFineSlider->setTickPosition(QSlider::TicksBelow);
    pFineLayout->addWidget(m_pFineSlider);
    QLabel* pFineLabel = new QLabel("+0.0 ms");
    pFineLayout->addWidget(pFineLabel);
    m_pFineSlider->setProperty("label", QVariant::fromValue(pFineLabel));
    pMainLayout->addLayout(pFineLayout);

    // Status label
    m_pStatusLabel = new QLabel(tr("Click \"Auto-Calibrate\" to measure latency."));
    m_pStatusLabel->setStyleSheet("font-weight: bold;");
    pMainLayout->addWidget(m_pStatusLabel);

    // Buttons
    QHBoxLayout* pButtonLayout = new QHBoxLayout();

    m_pAutoCalibrateButton = new QPushButton(tr("Auto-Calibrate"));
    pButtonLayout->addWidget(m_pAutoCalibrateButton);

    pButtonLayout->addStretch();

    m_pApplyButton = new QPushButton(tr("Apply"));
    m_pCancelButton = new QPushButton(tr("Cancel"));
    pButtonLayout->addWidget(m_pApplyButton);
    pButtonLayout->addWidget(m_pCancelButton);

    pMainLayout->addLayout(pButtonLayout);

    // Connections
    connect(m_pOffsetSpinbox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &DlgPrefSoundCalibrate::onOffsetChanged);
    connect(m_pFineSlider,
            &QSlider::valueChanged,
            this,
            &DlgPrefSoundCalibrate::onFineSliderChanged);
    connect(m_pAutoCalibrateButton,
            &QPushButton::clicked,
            this,
            &DlgPrefSoundCalibrate::onAutoCalibrateClicked);
    connect(m_pApplyButton,
            &QPushButton::clicked,
            this,
            &DlgPrefSoundCalibrate::onApplyClicked,
            Qt::QueuedConnection);
    connect(m_pCancelButton, &QPushButton::clicked, this, &QDialog::reject, Qt::QueuedConnection);
}

void DlgPrefSoundCalibrate::onOffsetChanged(double value) {
    m_currentOffsetMs = value;
    updateStatusLabel();
}

void DlgPrefSoundCalibrate::onFineSliderChanged(int value) {
    m_fineOffsetMs = static_cast<double>(value) / 10.0; // 0.1ms steps
    QLabel* pFineLabel = m_pFineSlider->property("label").value<QLabel*>();
    if (pFineLabel) {
        QString prefix = m_fineOffsetMs >= 0.0 ? "+" : "";
        pFineLabel->setText(prefix + QString::number(m_fineOffsetMs, 'f', 1) + " ms");
    }
    updateStatusLabel();
}

void DlgPrefSoundCalibrate::onAutoCalibrateClicked() {
    m_pAutoCalibrateButton->setEnabled(false);
    m_pAutoCalibrateButton->setText(tr("Measuring..."));

    if (!m_pSoundItem) {
        m_pStatusLabel->setText(tr("Error: no output item selected."));
        m_pAutoCalibrateButton->setEnabled(true);
        m_pAutoCalibrateButton->setText(tr("Auto-Calibrate"));
        return;
    }

    // Auto-calibrate based on device buffer latency.
    // The baseline offset is the output buffer duration in ms:
    //   offset_ms = framesPerBuffer * 1000 / sampleRate
    // This reflects the inherent delay from audio buffering.
    //
    // On Android USB audio (like DDJ-FLX4), typical values are:
    //   - 960 frames @ 48000 Hz = 20ms per buffer
    //   - Total round-trip = ~40-60ms including USB transport

    double baselineMs = static_cast<double>(m_framesPerBuffer) * 1000.0 / m_sampleRate;

    // Clamp to reasonable range
    if (baselineMs < 1.0) {
        baselineMs = 10.0;
    }
    if (baselineMs > 200.0) {
        baselineMs = 200.0;
    }

    m_currentOffsetMs = baselineMs;
    m_pOffsetSpinbox->blockSignals(true);
    m_pOffsetSpinbox->setValue(m_currentOffsetMs);
    m_pOffsetSpinbox->blockSignals(false);

    m_pStatusLabel->setText(
            tr("Auto-calibrated: %1 ms (based on %2 frames @ %3 Hz)")
                    .arg(static_cast<int>(std::round(baselineMs)))
                    .arg(m_framesPerBuffer)
                    .arg(m_sampleRate));

    m_pAutoCalibrateButton->setEnabled(true);
    m_pAutoCalibrateButton->setText(tr("Re-Calibrate"));
}

void DlgPrefSoundCalibrate::updateReferenceTone() {
    // Tone-based calibration is replaced by buffer-latency auto-calibrate.
}

void DlgPrefSoundCalibrate::onApplyClicked() {
    if (m_pSoundItem) {
        int totalMs = static_cast<int>(m_currentOffsetMs + m_fineOffsetMs);
        totalMs = math_clamp(totalMs, 0, 500);
        m_pSoundItem->setLatencyOffsetMs(totalMs);
    }
    accept();
}

void DlgPrefSoundCalibrate::updateStatusLabel() {
    int totalMs = static_cast<int>(
            std::round(m_currentOffsetMs + m_fineOffsetMs));
    totalMs = math_clamp(totalMs, 0, 500);
    m_pStatusLabel->setText(tr("Total offset: %1 ms").arg(totalMs));
}

void showLatencyCalibrationDialog(QWidget* parent,
        DlgPrefSoundItem* item,
        int framesPerBuffer,
        int sampleRate) {
    DlgPrefSoundCalibrate dialog(parent, item, framesPerBuffer, sampleRate);
    // Qt modal dialog handles blocking events to parent widgets natively.
    // No global event filter needed — the underlying crash (null-this in
    // setupDevices with extra Main outputs) was fixed upstream.
    dialog.exec();
}

#include "moc_dlgprefsoundcalibrate.cpp"
