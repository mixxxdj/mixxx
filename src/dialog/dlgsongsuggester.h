#pragma once

#include <QDialog>
#include <QList>

#include "track/track_decl.h"

class PlayerManager;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QListWidget;
class TrackCollectionManager;

/// Dialog that suggests library tracks matching the tempo, energy
/// (loudness), language, and fame (play count) of a reference track.
class DlgSongSuggester : public QDialog {
    Q_OBJECT
  public:
    DlgSongSuggester(
            TrackCollectionManager* pTrackCollectionManager,
            PlayerManager* pPlayerManager,
            QWidget* pParent = nullptr);

  private slots:
    void slotSuggest();
    void slotUseCurrentTrack();

  private:
    void setReferenceTrack(const TrackPointer& pTrack);
    void prefillCriteria(const TrackPointer& pTrack);
    void loadSelectedTrackToDeck(int deck);

    TrackCollectionManager* const m_pTrackCollectionManager;
    PlayerManager* const m_pPlayerManager;
    TrackPointer m_pReferenceTrack;
    QList<TrackPointer> m_matches;

    QLabel* m_pReferenceLabel;
    QLabel* m_pResultCountLabel;
    QDoubleSpinBox* m_pTargetBpmSpinBox;
    QDoubleSpinBox* m_pBpmToleranceSpinBox;
    QComboBox* m_pEnergyComboBox;
    QLineEdit* m_pLanguageEdit;
    QComboBox* m_pFameComboBox;
    QListWidget* m_pResultsList;
};