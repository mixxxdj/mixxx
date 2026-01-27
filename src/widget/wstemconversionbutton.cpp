#include "widget/wstemconversionbutton.h"

#include <QPaintEvent>
#include <QPainter>
#include <QTimer>

#include "stems/dlgstemconversion.h"

WStemConversionButton::WStemConversionButton(
        StemConversionManagerPointer pConversionManager,
        QWidget* parent)
        : QPushButton(parent),
          m_pConversionManager(pConversionManager),
          m_rotationAngle(0),
          m_pendingCount(0),
          m_isProcessing(false),
          m_pDialog(nullptr) {
    createUI();
    connectSignals();
    updateButtonState();
}

void WStemConversionButton::createUI() {
    setToolTip("Click to view stem conversion status");
    setMinimumWidth(50);
    setMinimumHeight(50);
    setMaximumWidth(60);
    setMaximumHeight(60);

    setStyleSheet(
            "QPushButton {"
            "    border-radius: 8px;"
            "    border: 2px solid #555;"
            "    background-color: #333;"
            "    color: white;"
            "    font-weight: bold;"
            "    font-size: 12px;"
            "}"
            "QPushButton:hover {"
            "    background-color: #444;"
            "    border: 2px solid #777;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #222;"
            "}");

    setText("STEM");

    connect(this, &QPushButton::clicked, this, &WStemConversionButton::onButtonClicked);
}

void WStemConversionButton::connectSignals() {
    if (!m_pConversionManager) {
        return;
    }

    connect(m_pConversionManager.get(),
            &StemConversionManager::conversionStarted,
            this,
            &WStemConversionButton::onConversionStarted);
    connect(m_pConversionManager.get(),
            &StemConversionManager::conversionCompleted,
            this,
            &WStemConversionButton::onConversionCompleted);
    connect(m_pConversionManager.get(),
            &StemConversionManager::conversionFailed,
            this,
            &WStemConversionButton::onConversionFailed);
    connect(m_pConversionManager.get(),
            &StemConversionManager::queueChanged,
            this,
            &WStemConversionButton::onQueueChanged);
}

void WStemConversionButton::paintEvent(QPaintEvent* event) {
    QPushButton::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw status indicator circle
    int size = 12;
    int x = width() - size - 4;
    int y = height() - size - 4;

    // Determine color based on state
    QColor indicatorColor = QColor(100, 100, 100); // Gray (idle)
    if (m_isProcessing) {
        indicatorColor = QColor(255, 165, 0); // Orange (processing)
    } else if (m_pendingCount > 0) {
        indicatorColor = QColor(255, 165, 0); // Orange (pending)
    }

    // Draw indicator circle
    painter.setBrush(indicatorColor);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(x, y, size, size);

    // If processing, draw rotation animation
    if (m_isProcessing) {
        painter.save();
        painter.translate(x + size / 2, y + size / 2);
        painter.rotate(m_rotationAngle);

        painter.setPen(QPen(Qt::white, 2));
        painter.drawLine(0, -size / 2, 0, -size / 2 + 3);

        painter.restore();
    }

    // Draw pending count if any
    if (m_pendingCount > 0) {
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 7, QFont::Bold));
        painter.drawText(x, y, size, size, Qt::AlignCenter, QString::number(m_pendingCount));
    }
}

void WStemConversionButton::onConversionStarted(TrackId trackId, const QString& trackTitle) {
    Q_UNUSED(trackId);
    Q_UNUSED(trackTitle);
    m_isProcessing = true;
    startAnimation();
    updateButtonState();
}

void WStemConversionButton::onConversionCompleted(TrackId trackId) {
    Q_UNUSED(trackId);
    updateButtonState();
}

void WStemConversionButton::onConversionFailed(TrackId trackId, const QString& errorMessage) {
    Q_UNUSED(trackId);
    Q_UNUSED(errorMessage);
    updateButtonState();
}

void WStemConversionButton::onQueueChanged(int pendingCount) {
    m_pendingCount = pendingCount;
    updateButtonState();
    update();
}

void WStemConversionButton::onAnimationTick() {
    m_rotationAngle = (m_rotationAngle + 6) % 360;
    update();
}

void WStemConversionButton::onButtonClicked() {
    if (!m_pConversionManager) {
        return;
    }

    // Si la finestra ja existeix, només la mostrem
    if (m_pDialog) {
        m_pDialog->raise();
        m_pDialog->activateWindow();
        return;
    }

    // Crear la finestra com a finestra modeless (no bloquejant)
    m_pDialog = new DlgStemConversion(m_pConversionManager, this);

    // Connectar el senyal de tancament per netejar el punter
    connect(m_pDialog, &QDialog::finished, this, [this]() {
        m_pDialog = nullptr;
    });

    // Mostrar la finestra (no bloqueja)
    m_pDialog->show();
}

void WStemConversionButton::updateButtonState() {
    if (!m_pConversionManager) {
        return;
    }

    auto currentConversion = m_pConversionManager->getCurrentConversion();
    m_isProcessing = currentConversion.has_value();

    if (m_isProcessing) {
        setToolTip(QString("Converting: %1\nClick for details")
                        .arg(currentConversion->trackTitle));
        startAnimation();
    } else if (m_pendingCount > 0) {
        setToolTip(QString("Pending conversions: %1\nClick for details")
                        .arg(m_pendingCount));
        stopAnimation();
    } else {
        setToolTip("No conversions in progress\nClick to view history");
        stopAnimation();
    }

    update();
}

void WStemConversionButton::startAnimation() {
    // Create timer for animation
    QTimer* pTimer = new QTimer(this);
    connect(pTimer, &QTimer::timeout, this, &WStemConversionButton::onAnimationTick);
    pTimer->start(50); // Update every 50ms
}

void WStemConversionButton::stopAnimation() {
    m_rotationAngle = 0;
    update();
}

#include "moc_wstemconversionbutton.cpp"
