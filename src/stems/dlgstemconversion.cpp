#include "stems/dlgstemconversion.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QListWidgetItem>
#include <QColor>
#include <QApplication>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include "widget/dlgstemconversionoptions.h"
#include "track/track.h"

DlgStemConversion::DlgStemConversion(
        StemConversionManagerPointer pConversionManager,
        QWidget* parent)
        : QDialog(parent),
          m_pConversionManager(pConversionManager) {
    setWindowTitle("Stem Conversion Status");
    setMinimumWidth(600);
    setMinimumHeight(400);

    // Window modeless (non-blocking) - IMPORTANT: This allows the window to be closed
    setWindowModality(Qt::NonModal);
    
    // Allow the window to be closed with the X button
    setWindowFlags(windowFlags() | Qt::WindowCloseButtonHint);

    createUI();
    connectSignals();
    updateConversionList();

    // Show window (non-blocking)
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

    // Phase/Status message label (NEW)
    m_pStatusLabel = new QLabel("Waiting for conversion...", this);
    m_pStatusLabel->setStyleSheet("color: #0066CC; font-weight: bold;");  // Blue, bold
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

    m_pConvertNewButton = new QPushButton("Convert New Track", this);
    connect(m_pConvertNewButton, &QPushButton::clicked, this, &DlgStemConversion::onConvertNewTrack);
    pButtonLayout->addWidget(m_pConvertNewButton);

    m_pCloseButton = new QPushButton("Close", this);
    // Use reject() instead of accept() to close the dialog without blocking
    connect(m_pCloseButton, &QPushButton::clicked, this, &QDialog::reject);
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
    m_pStatusLabel->setText("⏳ Initializing conversion...");
    QApplication::processEvents();  // Update UI
}

void DlgStemConversion::onConversionProgress(TrackId trackId, float progress, const QString& message) {
    Q_UNUSED(trackId);
    m_pProgressBar->setValue(static_cast<int>(progress * 100));

    // Format the message with progress indicator and emoji
    QString displayMessage = message;

    // Add progress percentage
    displayMessage += QString(" (%1%)").arg(static_cast<int>(progress * 100));

    // Add phase emoji based on progress
    if (progress < 0.2f) {
        displayMessage = "🔍 " + displayMessage;  // Searching/Initializing
    } else if (progress < 0.5f) {
        displayMessage = "🎵 " + displayMessage;  // Demucs separation
    } else if (progress < 0.7f) {
        displayMessage = "🔄 " + displayMessage;  // Converting to M4A
    } else if (progress < 0.9f) {
        displayMessage = "📦 " + displayMessage;  // Creating container
    } else {
        displayMessage = "✅ " + displayMessage;  // Finalizing
    }

    m_pStatusLabel->setText(displayMessage);
    QApplication::processEvents();  // Update UI
}

void DlgStemConversion::onConversionCompleted(TrackId trackId, const QString& trackTitle) {
    Q_UNUSED(trackId);
    m_pProgressBar->setValue(100);
    m_pStatusLabel->setText("✅ " + trackTitle + " - Conversion completed successfully! (100%)");
    updateConversionList();
    QApplication::processEvents();  // Update UI
}

void DlgStemConversion::onConversionFailed(TrackId trackId, const QString& trackTitle, const QString& errorMessage) {
    Q_UNUSED(trackId);
    m_pStatusLabel->setText(QString("❌ Error converting %1: %2").arg(trackTitle, errorMessage));
    m_pStatusLabel->setStyleSheet("color: #CC0000; font-weight: bold;");  // Red, bold
    updateConversionList();
    QApplication::processEvents();  // Update UI
}

void DlgStemConversion::onQueueChanged(int pendingCount) {
    if (pendingCount == 0) {
        m_pCurrentTrackLabel->setText("No conversion in progress");
        m_pStatusLabel->setStyleSheet("color: #0066CC; font-weight: bold;");  // Reset to blue
    }
    updateConversionList();
    QApplication::processEvents();  // Update UI
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
        QString itemText = QString("%1 - %2")
                .arg(status.trackTitle, stateText);

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

void DlgStemConversion::onConvertNewTrack() {
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select Audio File to Convert",
        QStandardPaths::writableLocation(QStandardPaths::MusicLocation),
        "Audio Files (*.mp3 *.wav *.flac *.m4a *.ogg)");

    if (filePath.isEmpty()) {
        return; // User canceled
    }

    // Create a temporary track object
    TrackPointer pTrack = Track::newTemporary(filePath);
    if (!pTrack) {
        QMessageBox::warning(this, "Error", "Could not load the selected track.");
        return;
    }

    // Open the options dialog
    DlgStemConversionOptions optionsDialog(pTrack->getLocation(), this);
    if (optionsDialog.exec() == QDialog::Accepted) {
        StemConverter::Resolution resolution;
        if (optionsDialog.getSelectedResolution() == DlgStemConversionOptions::Resolution::High) {
            resolution = StemConverter::Resolution::High;
        } else {
            resolution = StemConverter::Resolution::Low;
        }
        m_pConversionManager->convertTrack(pTrack, resolution);
    }
}

#include "moc_dlgstemconversion.cpp"
