#include "stems/dlgstemconversion.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QListWidgetItem>
#include <QColor>
#include <QApplication>

DlgStemConversion::DlgStemConversion(
        StemConversionManagerPointer pConversionManager,
        QWidget* parent)
        : QDialog(parent),
          m_pConversionManager(pConversionManager) {
    setWindowTitle("Stem Conversion Status");
    setMinimumWidth(600);
    setMinimumHeight(400);

    // Fer la finestra modeless (no bloquejant)
    setWindowModality(Qt::NonModal);

    createUI();
    connectSignals();
    updateConversionList();

    // Mostrar la finestra
    show();
}

void DlgStemConversion::createUI() {
    QVBoxLayout* pMainLayout = new QVBoxLayout(this);

    // Current conversion section
    QGroupBox* pCurrentGroup = new QGroupBox("Current Conversion", this);
    QVBoxLayout* pCurrentLayout = new QVBoxLayout(pCurrentGroup);

    m_pCurrentTrackLabel = new QLabel("No conversion in progress", this);
    pCurrentLayout->addWidget(m_pCurrentTrackLabel);

    m_pProgressBar = new QProgressBar(this);
    m_pProgressBar->setRange(0, 100);
    m_pProgressBar->setValue(0);
    pCurrentLayout->addWidget(m_pProgressBar);

    m_pStatusLabel = new QLabel("Waiting for conversion...", this);
    pCurrentLayout->addWidget(m_pStatusLabel);

    pMainLayout->addWidget(pCurrentGroup);

    // Conversion history section
    QGroupBox* pHistoryGroup = new QGroupBox("Conversion History", this);
    QVBoxLayout* pHistoryLayout = new QVBoxLayout(pHistoryGroup);

    m_pConversionListWidget = new QListWidget(this);
    pHistoryLayout->addWidget(m_pConversionListWidget);

    pMainLayout->addWidget(pHistoryGroup);

    // Buttons
    QHBoxLayout* pButtonLayout = new QHBoxLayout();
    m_pClearHistoryButton = new QPushButton("Clear History", this);
    connect(m_pClearHistoryButton, &QPushButton::clicked, this, &DlgStemConversion::onClearHistory);
    pButtonLayout->addWidget(m_pClearHistoryButton);

    pButtonLayout->addStretch();

    m_pCloseButton = new QPushButton("Close", this);
    connect(m_pCloseButton, &QPushButton::clicked, this, &QDialog::accept);
    pButtonLayout->addWidget(m_pCloseButton);

    pMainLayout->addLayout(pButtonLayout);
}

void DlgStemConversion::connectSignals() {
    if (!m_pConversionManager) {
        return;
    }

    connect(m_pConversionManager.get(), &StemConversionManager::conversionStarted,
            this, &DlgStemConversion::onConversionStarted);
    connect(m_pConversionManager.get(), &StemConversionManager::conversionProgress,
            this, &DlgStemConversion::onConversionProgress);
    connect(m_pConversionManager.get(), &StemConversionManager::conversionCompleted,
            this, &DlgStemConversion::onConversionCompleted);
    connect(m_pConversionManager.get(), &StemConversionManager::conversionFailed,
            this, &DlgStemConversion::onConversionFailed);
    connect(m_pConversionManager.get(), &StemConversionManager::queueChanged,
            this, &DlgStemConversion::onQueueChanged);
}

void DlgStemConversion::onConversionStarted(TrackId trackId, const QString& trackTitle) {
    Q_UNUSED(trackId);
    m_pCurrentTrackLabel->setText(QString("Converting: %1").arg(trackTitle));
    m_pProgressBar->setValue(0);
    m_pStatusLabel->setText("Starting conversion...");
    QApplication::processEvents();  // Actualizar la UI
}

void DlgStemConversion::onConversionProgress(TrackId trackId, float progress, const QString& message) {
    Q_UNUSED(trackId);
    m_pProgressBar->setValue(static_cast<int>(progress * 100));
    m_pStatusLabel->setText(message);
    QApplication::processEvents();  // Actualizar la UI
}

void DlgStemConversion::onConversionCompleted(TrackId trackId) {
    Q_UNUSED(trackId);
    m_pProgressBar->setValue(100);
    m_pStatusLabel->setText("Conversion completed successfully");
    updateConversionList();
    QApplication::processEvents();  // Actualizar la UI
}

void DlgStemConversion::onConversionFailed(TrackId trackId, const QString& errorMessage) {
    Q_UNUSED(trackId);
    m_pStatusLabel->setText(QString("Error: %1").arg(errorMessage));
    updateConversionList();
    QApplication::processEvents();  // Actualizar la UI
}

void DlgStemConversion::onQueueChanged(int pendingCount) {
    if (pendingCount == 0) {
        m_pCurrentTrackLabel->setText("No conversion in progress");
    }
    updateConversionList();
    QApplication::processEvents();  // Actualizar la UI
}

void DlgStemConversion::onClearHistory() {
    if (m_pConversionManager) {
        m_pConversionManager->clearConversionHistory();
        updateConversionList();
    }
}

void DlgStemConversion::updateConversionList() {
    m_pConversionListWidget->clear();

    if (!m_pConversionManager) {
        return;
    }

    QList<StemConversionManager::ConversionStatus> history =
            m_pConversionManager->getConversionHistory();

    for (const auto& status : history) {
        QString stateText = getStateDisplayText(status.state);
        QString itemText = QString("%1 - %2 (%3%)")
                .arg(status.trackTitle,
                     stateText,
                     QString::number(static_cast<int>(status.progress * 100)));

        QListWidgetItem* pItem = new QListWidgetItem(itemText, m_pConversionListWidget);

        // Color based on state
        if (status.state == StemConverter::ConversionState::Completed) {
            pItem->setBackground(QColor(144, 238, 144)); // Light green
        } else if (status.state == StemConverter::ConversionState::Failed) {
            pItem->setBackground(QColor(255, 160, 160)); // Light red
        } else if (status.state == StemConverter::ConversionState::Processing) {
            pItem->setBackground(QColor(173, 216, 230)); // Light blue
        }
    }
}

QString DlgStemConversion::getStateDisplayText(StemConverter::ConversionState state) const {
    switch (state) {
        case StemConverter::ConversionState::Idle:
            return "Idle";
        case StemConverter::ConversionState::Processing:
            return "Processing";
        case StemConverter::ConversionState::Completed:
            return "Completed";
        case StemConverter::ConversionState::Failed:
            return "Failed";
        default:
            return "Unknown";
    }
}

#include "moc_dlgstemconversion.cpp"
