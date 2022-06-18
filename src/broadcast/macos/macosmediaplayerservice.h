#pragma once

#include <QObject.h>

#include "library/coverart.h"
#include "mixer/playermanager.h"
#include "util/cache.h"

// TODO: Implement ScrobblingService once merged
class MacOSMediaPlayerService : public QObject {
    Q_OBJECT
  public:
    MacOSMediaPlayerService();
  public slots:
    void slotBroadcastCurrentTrack(TrackPointer pTrack); // TODO: override
    void slotAllTracksPaused();                          // TODO: override
  private slots:
    void slotCoverFound(const QObject* pRequestor,
            const CoverInfo& coverInfo,
            const QPixmap& pixmap);
};
