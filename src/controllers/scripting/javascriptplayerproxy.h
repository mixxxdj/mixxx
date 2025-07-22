#pragma once

#include "mixer/basetrackplayer.h"
#include "track/track.h"

#define PROPERTY_IMPL_GETTER(NAMESPACE, TYPE, NAME, GETTER) \
    TYPE NAMESPACE::GETTER() const {                        \
        const TrackPointer pTrack = m_pCurrentTrack;        \
        if (pTrack == nullptr) {                            \
            return TYPE();                                  \
        }                                                   \
        return pTrack->GETTER();                            \
    }

class JavascriptPlayerProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString artist READ getArtist NOTIFY artistChanged)
    Q_PROPERTY(QString title READ getTitle NOTIFY titleChanged)
    Q_PROPERTY(QString album READ getAlbum NOTIFY albumChanged)
    Q_PROPERTY(QString albumArtist READ getAlbumArtist NOTIFY albumArtistChanged)
    Q_PROPERTY(QString genre READ getGenre STORED false NOTIFY genreChanged)
    Q_PROPERTY(QString composer READ getComposer NOTIFY composerChanged)
    Q_PROPERTY(QString grouping READ getGrouping NOTIFY groupingChanged)
    Q_PROPERTY(QString year READ getYear NOTIFY yearChanged)
    Q_PROPERTY(QString trackNumber READ getTrackNumber NOTIFY trackNumberChanged)
    Q_PROPERTY(QString trackTotal READ getTrackTotal NOTIFY trackTotalChanged)
  public:
    explicit JavascriptPlayerProxy(BaseTrackPlayer* pTrackPlayer, QObject* parent);

    QString getTitle() const;
    QString getArtist() const;
    QString getAlbum() const;
    QString getAlbumArtist() const;
    QString getGenre() const;
    QString getComposer() const;
    QString getGrouping() const;
    QString getYear() const;
    QString getTrackNumber() const;
    QString getTrackTotal() const;

  public slots:
    void slotTrackLoaded(TrackPointer pTrack);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);

  signals:
    void trackUnloaded();
    void albumChanged(QString newAlbum);
    void titleChanged(QString newTitle);
    void artistChanged(QString newArtist);
    void albumArtistChanged(QString newAlbumArtist);
    void genreChanged(QString newGenre);
    void composerChanged(QString newComposer);
    void groupingChanged(QString grouping);
    void yearChanged(QString newYear);
    void trackNumberChanged(QString newTrackNumber);
    void trackTotalChanged(QString newTrackTotal);

  protected:
    void disconnectTrack();
    QPointer<BaseTrackPlayer> m_pTrackPlayer;
    TrackPointer m_pCurrentTrack;
};
