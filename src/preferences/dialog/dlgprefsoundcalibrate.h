#pragma once

#include <QDialog>
#include <QTimer>

#include "preferences/dialog/dlgprefsounditem.h"

/**
 * Calibration dialog for fine-tuning per-output latency offset.
 *
 * Inspired by the osu! framework's BeatmapOffsetControl:
 * - Plays a periodic sync click through the output device
 * - User adjusts offset until the click aligns with what they hear
 * - Provides both auto-calibrate (loopback detection) and manual modes
 */
class DlgPrefSoundCalibrate : public QDialog {
    Q_OBJECT
  public:
    explicit DlgPrefSoundCalibrate(QWidget* parent,
            DlgPrefSoundItem* pSoundItem);
    ~DlgPrefSoundCalibrate() override = default;

  private slots:
    void updateReferenceTone();
    void onOffsetChanged(double value);
    void onFineSliderChanged(int value);
    void onAutoCalibrateClicked();
    void onPlayToneToggled(bool checked);
    void onApplyClicked();

  private:
    void setupUi();
    void updateStatusLabel();

    DlgPrefSoundItem* m_pSoundItem;
    QTimer* m_pSyncTimer;
    double m_currentOffsetMs;
    double m_fineOffsetMs;
    bool m_playingTone;

    class QLabel* m_pStatusLabel;
    class QDoubleSpinBox* m_pOffsetSpinbox;
    class QSlider* m_pFineSlider;
    class QPushButton* m_pAutoCalibrateButton;
    class QPushButton* m_pPlayToneButton;
    class QPushButton* m_pApplyButton;
    class QPushButton* m_pCancelButton;
    class QLabel* m_pExplanationLabel;
};

/// Show the latency calibration dialog for a given output item.
/// Defined in dlgprefsoundcalibrate.cpp to avoid AUTOMOC conflicts.
void showLatencyCalibrationDialog(QWidget* parent, class DlgPrefSoundItem* item);
