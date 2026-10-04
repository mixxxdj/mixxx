#pragma once

#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QString>

#include "dialog/autorecommendbot.h"

class QCheckBox;
class QDoubleSpinBox;
class QSpinBox;

/// Control panel for the Auto-Recommendation bot. When enabled, the
/// bot keeps the AutoDJ queue topped up with tracks that best match
/// the currently playing track, as determined by the recommendation
/// algorithm in autorecommendationengine. Optionally, the bot also
/// preloads the top recommendation into an idle deck when a deck
/// stops playing, and sequences the queue for smooth fades using
/// each track's duration and BPM.
class DlgAutoRecommend : public QDialog {
    Q_OBJECT

  public:
    DlgAutoRecommend(
            AutoRecommendBot* pBot,
            QWidget* pParent = nullptr);

  private slots:
    void slotEnabledToggled(bool checked);
    void slotDeckPreloadingToggled(bool checked);
    void slotTransitionAwareToggled(bool checked);
    void slotQueueSizeChanged(int value);
    void slotWeightsChanged();
    void slotRecommended(int count);
    void slotTrackPreloaded(const QString& deckGroup, const QString& trackTitle);

  private:
    AutoRecommendBot* const m_pBot;

    QCheckBox* m_pEnabledCheckBox;
    QCheckBox* m_pDeckPreloadingCheckBox;
    QCheckBox* m_pTransitionAwareCheckBox;
    QSpinBox* m_pQueueSizeSpinBox;
    QDoubleSpinBox* m_pTempoWeightSpinBox;
    QDoubleSpinBox* m_pEnergyWeightSpinBox;
    QDoubleSpinBox* m_pKeyWeightSpinBox;
    QDoubleSpinBox* m_pFameWeightSpinBox;
    QLabel* m_pStatusLabel;

    QString m_lastPreloadedDeckGroup;
    QString m_lastPreloadedTrackTitle;
    int m_lastRecommendedCount = 0;
};
