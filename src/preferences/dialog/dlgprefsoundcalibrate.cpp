#include "preferences/dialog/dlgprefsoundcalibrate.h"

#include <QApplication>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QTouchEvent>
#include <QVBoxLayout>

#include "util/math.h"

DlgPrefSoundCalibrate::DlgPrefSoundCalibrate(QWidget* parent,
        DlgPrefSoundItem* pSoundItem)
        : QDialog(parent),
          m_pSoundItem(pSoundItem),
          m_currentOffsetMs(0.0),
          m_fineOffsetMs(0.0),
          m_playingTone(false),
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
            tr("This tool the output devices by playing "
               "a rhythmic click. Adjust the offset until the click aligns "
               "with the audio you hear from your other output.<br><br>"
               "<b>Manual mode:</b> Click \"Play Click\" to hear a periodic "
               "beep on this output. Adjust the spins until it syncs with "
               "your other output.<br>"
               "<b>Auto-calibrate:</b> If you have a loopback connection "
               "(output cable connected to input), it can estimate the "
               "offset automatically."));
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
    m_pStatusLabel = new QLabel(tr("Press \"Play Click\" to start."));
    m_pStatusLabel->setStyleSheet("font-weight: bold;");
    pMainLayout->addWidget(m_pStatusLabel);

    // Buttons
    QHBoxLayout* pButtonLayout = new QHBoxLayout();
    m_pPlayToneButton = new QPushButton(tr("Play Click"));
    m_pPlayToneButton->setCheckable(true);
    pButtonLayout->addWidget(m_pPlayToneButton);

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
    connect(m_pPlayToneButton,
            &QPushButton::toggled,
            this,
            &DlgPrefSoundCalibrate::onPlayToneToggled,
            Qt::QueuedConnection);
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

void DlgPrefSoundCalibrate::onPlayToneToggled(bool checked) {
    m_playingTone = checked;
    if (checked) {
        m_pSyncTimer->start();
        updateReferenceTone(); // immediate
        m_pPlayToneButton->setText(tr("Stop"));
        m_pStatusLabel->setText(tr("Playing sync click every second..."));
    } else {
        m_pSyncTimer->stop();
        m_pPlayToneButton->setText(tr("Play Click"));
        m_pStatusLabel->setText(tr("Click stopped."));
    }
}

void DlgPrefSoundCalibrate::onAutoCalibrateClicked() {
    m_pAutoCalibrateButton->setEnabled(false);
    m_pStatusLabel->setText(tr(
            "Auto-calibrating... Play loopback click "
            "and record through input."));

    // TODO: Implement loopback detection once AudioLatencyCalibrator
    // is wired into the audio.
    // For now, set a placeholder value
    m_pAutoCalibrateButton->setEnabled(true);
    m_pStatusLabel->setText(tr(
            "Auto-calibrate not yet available. "
            "Use manual mode: play click and adjust."));
}

void DlgPrefSoundCalibrate::updateReferenceTone() {
    // This method is called every second by the timer.
    // In a full implementation, this would send a click/pulse to
    // the sound output device via a dedicated sync channel.
    // For now, we use the system beep as a visual/audible cue.
    QApplication::beep();
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
    m_pStatusLabel->setText(tr("Total offset: %1 ms. %2")
                    .arg(totalMs)
                    .arg(m_playingTone ? tr("Listening for sync...")
                                       : tr("Press Play Click to hear.")));
}

#include "moc_dlgprefsoundcalibrate.cpp"

void showLatencyCalibrationDialog(QWidget* parent, DlgPrefSoundItem* item) {
    DlgPrefSoundCalibrate dialog(parent, item);
    // Install a global event filter during QDialog::exec() to block all
    // mouse/touch events except those targeting the calibration dialog.
    // On Android, the QPA can deliver touch events to parent-window widgets
    // behind modal dialogs because they're queued before the dialog opens.
    // The filter intercepts QApplication-level notify() and swallows any
    // mouse/touch event whose target is NOT the dialog or its children.
    class TouchBlockFilter : public QObject {
      public:
        explicit TouchBlockFilter(DlgPrefSoundCalibrate* pDialog)
                : m_pDialog(pDialog) {
        }

      protected:
        bool eventFilter(QObject* pObj, QEvent* pEvent) override {
            // Block mouse/touch events for all targets EXCEPT the dialog
            switch (pEvent->type()) {
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonRelease:
            case QEvent::MouseButtonDblClick:
            case QEvent::TouchBegin:
            case QEvent::TouchUpdate:
            case QEvent::TouchEnd:
                // Allow events targeting the dialog or its children
                for (QObject* p = pObj; p; p = p->parent()) {
                    if (p == m_pDialog) {
                        return false;
                    }
                }
                return true; // block
            default:
                return false;
            }
        }

      private:
        DlgPrefSoundCalibrate* m_pDialog;
    };
    TouchBlockFilter blocker(&dialog);
    qApp->installEventFilter(&blocker);
    dialog.exec();
    qApp->removeEventFilter(&blocker);
}
