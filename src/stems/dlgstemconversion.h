#pragma once

#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QListWidget>
#include <QPushButton>
#include <memory>

#include "stems/stemconversionmanager.h"

/// Dialog that displays the status of stem conversions
class DlgStemConversion : public QDialog {
    Q_OBJECT

  public:
    explicit DlgStemConversion(
            StemConversionManagerPointer pConversionManager,
            QWidget* parent = nullptr);
    ~DlgStemConversion() override = default;

  private slots:
    /// Updates the UI when a conversion starts
    void onConversionStarted(TrackId trackId, const QString& trackTitle);

    /// Updates the UI when conversion progresses
    void onConversionProgress(TrackId trackId, float progress, const QString& message);

    /// Updates the UI when a conversion completes
    void onConversionCompleted(TrackId trackId, const QString& trackTitle);

    /// Updates the UI when a conversion fails
    void onConversionFailed(TrackId trackId, const QString& trackTitle, const QString& errorMessage);

    /// Updates the UI when the queue changes
    void onQueueChanged(int pendingCount);

    /// Clears the conversion history
    void onClearHistory();

    /// Updates the conversion list
    void updateConversionList();

    /// Opens a file dialog to convert a new track
    void onConvertNewTrack();

  private:
    void createUI();
    void connectSignals();
    QString getStateDisplayText(StemConverter::ConversionState state) const;

    StemConversionManagerPointer m_pConversionManager;

    // Widgets
    QLabel* m_pCurrentTrackLabel;
    QProgressBar* m_pProgressBar;
    QLabel* m_pStatusLabel;
    QListWidget* m_pConversionListWidget;
    QPushButton* m_pClearHistoryButton;
    QPushButton* m_pConvertNewButton;
    QPushButton* m_pCloseButton;
};
