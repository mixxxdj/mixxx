#pragma once

#include <QBuffer>

#include "mixer/basetrackplayer.h"
#include "track/track.h"
#include "waveform/waveform.h"

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
    Q_PROPERTY(bool isLoaded READ isLoaded NOTIFY trackChanged)
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

    Q_PROPERTY(int waveformLength READ getWaveformLength NOTIFY waveformLengthChanged)
    Q_PROPERTY(QString waveformTexture READ getWaveformTexture NOTIFY waveformTextureChanged)
    Q_PROPERTY(int waveformTextureSize READ getWaveformTextureSize NOTIFY
                    waveformTextureSizeChanged)
    Q_PROPERTY(int waveformTextureStride READ getWaveformTextureStride NOTIFY
                    waveformTextureStrideChanged)

  public:
    explicit JavascriptPlayerProxy(BaseTrackPlayer* pTrackPlayer, QObject* parent);

    bool isLoaded() const;
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

    int getWaveformLength() const;
    QString getWaveformTexture() const;
    int getWaveformTextureSize() const;
    int getWaveformTextureStride() const;

  public slots:
    void slotTrackLoaded(TrackPointer pTrack);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void slotTrackChanged();
    void slotWaveformChanged();

  signals:
    void trackLoading();
    void trackLoaded();
    void trackUnloaded();
    void trackChanged();

    void albumChanged();
    void titleChanged();
    void artistChanged();
    void albumArtistChanged();
    void genreChanged();
    void composerChanged();
    void groupingChanged();
    void yearChanged();
    void trackNumberChanged();
    void trackTotalChanged();

    void waveformLengthChanged();
    void waveformTextureChanged();
    void waveformTextureSizeChanged();
    void waveformTextureStrideChanged();

  protected:
    virtual void subclassesSlotTrackLoaded() {
    }
    virtual void subclassesSlotTrackChanged() {
    }
    std::vector<WaveformFilteredData> m_waveformData;
    QImage m_waveformTexture;
    QPointer<BaseTrackPlayer> m_pTrackPlayer;
    TrackPointer m_pCurrentTrack;
};
