#pragma once

#include <QObject>

#include "track/track_decl.h"

class BaseTrackPlayer;
class CoverInfo;
QT_FORWARD_DECLARE_CLASS(QPixmap);

namespace mixxx {

/// Follows the track that is loaded in a player and loads its cover art.
/// The watcher tells when the cover is outdated, and its owner decides
/// whether and when to request the new cover.
class PlayerCoverArtWatcher : public QObject {
    Q_OBJECT
  public:
    /// Without a player the track is set with slotLoadTrack().
    PlayerCoverArtWatcher(BaseTrackPlayer* pPlayer, QObject* pParent);
    ~PlayerCoverArtWatcher() override = default;

    const TrackPointer& loadedTrack() const {
        return m_pLoadedTrack;
    }

    /// Requests the cover of the loaded track. coverFound() is emitted when
    /// it is available. Does nothing if no track is loaded.
    void requestCover();

    /// Returns true if a cover that CoverArtCache has found is the cover of
    /// a track. CoverArtCache answers requests for the same image with the
    /// cover info of the track that has requested it first, so the cover
    /// can belong to another track with the same image.
    static bool isCoverOfTrack(const CoverInfo& foundCover, const CoverInfo& trackCover);

  public slots:
    void slotLoadTrack(TrackPointer pTrack);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);

  signals:
    /// The loaded track has been replaced or unloaded. A cover that was
    /// found before does not belong to it.
    void trackChanged();
    /// A track has been loaded or the cover art of the loaded track has
    /// changed.
    void coverOutdated();
    /// The requested cover of the loaded track. The pixmap is null if the
    /// track has no cover art.
    void coverFound(const QPixmap& cover);

  private slots:
    void slotCoverFound(
            const QObject* pRequester,
            const CoverInfo& coverInfo,
            const QPixmap& pixmap);

  private:
    void disconnectLoadedTrack();

    TrackPointer m_pLoadedTrack;
};

} // namespace mixxx
