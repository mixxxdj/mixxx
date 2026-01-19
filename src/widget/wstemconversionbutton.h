#pragma once

#include <QPushButton>
#include <memory>

#include "stems/stemconversionmanager.h"

class DlgStemConversion;

/// Dynamic button that shows the status of stem conversions
class WStemConversionButton : public QPushButton {
    Q_OBJECT

  public:
    explicit WStemConversionButton(
            StemConversionManagerPointer pConversionManager,
            QWidget* parent = nullptr);
    ~WStemConversionButton() override = default;

  protected:
    void paintEvent(QPaintEvent* event) override;

  private slots:
    /// Updates the button when a conversion starts
    void onConversionStarted(TrackId trackId, const QString& trackTitle);

    /// Updates the button when a conversion completes
    void onConversionCompleted(TrackId trackId);

    /// Updates the button when a conversion fails
    void onConversionFailed(TrackId trackId, const QString& errorMessage);

    /// Updates the button when the queue changes
    void onQueueChanged(int pendingCount);

    /// Updates the rotation animation
    void onAnimationTick();

    /// Opens the conversion dialog
    void onButtonClicked();

  private:
    void createUI();
    void connectSignals();
    void updateButtonState();
    void startAnimation();
    void stopAnimation();

    StemConversionManagerPointer m_pConversionManager;

    // Animation
    int m_rotationAngle;
    int m_pendingCount;
    bool m_isProcessing;

    // Dialog pointer (non-blocking)
    DlgStemConversion* m_pDialog;
};
