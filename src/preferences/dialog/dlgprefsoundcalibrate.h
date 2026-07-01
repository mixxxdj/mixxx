#pragma once

#include <QDialog>
#include <QTimer>

#include "preferences/dialog/dlgprefsounditem.h"

/**
 * Calibration dialog for per-output latency offset.
 *
 * Uses device buffer latency as the baseline for auto-calibration.
 * The user can fine-tune with the coarse spinbox and fine slider.
 */
class DlgPrefSoundCalibrate : public QDialog {
    Q_OBJECT
  public:
    explicit DlgPrefSoundCalibrate(QWidget* parent,
            DlgPrefSoundItem* pSoundItem,
            int framesPerBuffer = 1024,
            int sampleRate = 44100);
    ~DlgPrefSoundCalibrate() override = default;

  private slots:
    void updateReferenceTone();
    void onOffsetChanged(double value);
    void onFineSliderChanged(int value);
    void onAutoCalibrateClicked();
    void onApplyClicked();

  private:
    void setupUi();
    void updateStatusLabel();

    DlgPrefSoundItem* m_pSoundItem;
    QTimer* m_pSyncTimer;
    double m_currentOffsetMs;
    double m_fineOffsetMs;
    bool m_playingTone;
    int m_framesPerBuffer;
    int m_sampleRate;

    class QLabel* m_pStatusLabel;
    class QDoubleSpinBox* m_pOffsetSpinbox;
    class QSlider* m_pFineSlider;
    class QPushButton* m_pAutoCalibrateButton;
    class QPushButton* m_pApplyButton;
    class QPushButton* m_pCancelButton;
    class QLabel* m_pExplanationLabel;
};

/// Show the latency calibration dialog for a given output item.
/// @param parent Parent widget (DlgPrefSound)
/// @param item The output item being calibrated
/// @param framesPerBuffer Optional: configured buffer frame count for auto-calibrate baseline
/// @param sampleRate Optional: configured sample rate for auto-calibrate baseline
/// Defined in dlgprefsoundcalibrate.cpp to avoid AUTOMOC conflicts.
void showLatencyCalibrationDialog(QWidget* parent,
        class DlgPrefSoundItem* item,
        int framesPerBuffer = 1024,
        int sampleRate = 44100);