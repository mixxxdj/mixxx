#pragma once

#include <QList>
#include <QObject>

#include "control/controlproxy.h"
#include "library/autodj/autodjprocessor.h"
#include "library/coverart.h"
#include "mixer/playermanager.h"
#include "util/cache.h"

// TODO: Implement ScrobblingService once merged
class MacOSMediaPlayerService : public QObject {
    Q_OBJECT
  public:
    MacOSMediaPlayerService(PlayerManagerInterface& pPlayerManager);
    ~MacOSMediaPlayerService() override;

  public slots:
    void slotBroadcastCurrentTrack(TrackPointer pTrack); // TODO: override
    void slotAllTracksPaused();                          // TODO: override

  private:
    QList<DeckAttributes*> m_deckAttributes;
    ControlProxy* m_pCPAutoDjEnabled;
    ControlProxy* m_pCPFadeNow;
    double m_lastSentPosition;

    DeckAttributes* getCurrentDeck();
    bool isCurrentDeck(DeckAttributes* attributes);

    bool togglePlayState();
    bool updatePlayState(bool playing);
    bool updatePlayPosition(double absolutePosition);
    bool skipToNextTrack();

  private slots:
    void slotPlayChanged(DeckAttributes* attributes, bool playing);
    void slotPlayPositionChanged(DeckAttributes* attributes, double position);

    void slotCoverFound(const QObject* pRequestor,
            const CoverInfo& coverInfo,
            const QPixmap& pixmap);
};
