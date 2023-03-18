#pragma once

#include <QObject.h>
#include <qlist.h>

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

    bool isCurrentDeck(DeckAttributes* attributes);

  private slots:
    void slotPlayChanged(DeckAttributes* attributes, bool playing);
    void slotPlayPositionChanged(DeckAttributes* attributes, double position);

    void slotCoverFound(const QObject* pRequestor,
            const CoverInfo& coverInfo,
            const QPixmap& pixmap);
};
