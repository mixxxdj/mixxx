#pragma once
#include <QColor>
#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QString>
#include <QUrl>

#include "mixer/basetrackplayer.h"
#include "qml/qmlbeatsmodel.h"
#include "qml/qmlcuesmodel.h"
#include "track/cueinfo.h"
#include "track/track.h"

namespace mixxx {
namespace qml {

class QmlPlayerProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isLoaded READ isLoaded NOTIFY trackChanged)
    Q_PROPERTY(QString artist READ getArtist WRITE setArtist NOTIFY artistChanged)
    Q_PROPERTY(QString title READ getTitle WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString album READ getAlbum WRITE setAlbum NOTIFY albumChanged)
    Q_PROPERTY(QString albumArtist READ getAlbumArtist WRITE setAlbumArtist
                    NOTIFY albumArtistChanged)
    Q_PROPERTY(QString genre READ getGenre STORED false NOTIFY genreChanged)
    Q_PROPERTY(QString composer READ getComposer WRITE setComposer NOTIFY composerChanged)
    Q_PROPERTY(QString grouping READ getGrouping WRITE setGrouping NOTIFY groupingChanged)
    Q_PROPERTY(QString year READ getYear WRITE setYear NOTIFY yearChanged)
    Q_PROPERTY(QString trackNumber READ getTrackNumber WRITE setTrackNumber
                    NOTIFY trackNumberChanged)
    Q_PROPERTY(QString trackTotal READ getTrackTotal WRITE setTrackTotal NOTIFY trackTotalChanged)
    Q_PROPERTY(QString comment READ getComment WRITE setComment NOTIFY commentChanged)
    Q_PROPERTY(QString keyText READ getKeyText WRITE setKeyText NOTIFY keyTextChanged)
    Q_PROPERTY(QColor color READ getColor WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QUrl coverArtUrl READ getCoverArtUrl NOTIFY coverArtUrlChanged)
    Q_PROPERTY(QUrl trackLocationUrl READ getTrackLocationUrl NOTIFY trackLocationUrlChanged)
    QML_NAMED_ELEMENT(Player)
    QML_UNCREATABLE("Only accessible via Mixxx.PlayerManager.getPlayer(group)")

    Q_PROPERTY(int waveformLength READ getWaveformLength NOTIFY waveformLengthChanged)
    Q_PROPERTY(QString waveformTexture READ getWaveformTexture NOTIFY waveformTextureChanged)
    Q_PROPERTY(int waveformTextureSize READ getWaveformTextureSize NOTIFY
                    waveformTextureSizeChanged)
    Q_PROPERTY(int waveformTextureStride READ getWaveformTextureStride NOTIFY
                    waveformTextureStrideChanged)

    Q_PROPERTY(mixxx::qml::QmlBeatsModel* beatsModel MEMBER m_pBeatsModel CONSTANT);
    Q_PROPERTY(mixxx::qml::QmlCuesModel* hotcuesModel MEMBER m_pHotcuesModel CONSTANT);

  public:
    explicit QmlPlayerProxy(BaseTrackPlayer* pTrackPlayer, QObject* parent = nullptr);

    bool isLoaded() const;
    QString getTrack() const;
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
    QString getComment() const;
    QString getKeyText() const;
    QColor getColor() const;
    QUrl getCoverArtUrl() const;
    QUrl getTrackLocationUrl() const;

    int getWaveformLength() const;
    QString getWaveformTexture() const;
    int getWaveformTextureSize() const;
    int getWaveformTextureStride() const;

    /// Needed for interacting with the raw track player object.
    BaseTrackPlayer* internalTrackPlayer() const {
        return m_pTrackPlayer;
    }

    Q_INVOKABLE void loadTrackFromLocation(const QString& trackLocation, bool play = false);
    Q_INVOKABLE void loadTrackFromLocationUrl(const QUrl& trackLocationUrl, bool play = false);

  public slots:
    void slotTrackLoaded(TrackPointer pTrack);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void slotTrackChanged();
    void slotWaveformChanged();
    void slotBeatsChanged();
    void slotHotcuesChanged();

    void setArtist(const QString& artist);
    void setTitle(const QString& title);
    void setAlbum(const QString& album);
    void setAlbumArtist(const QString& albumArtist);
    void setComposer(const QString& composer);
    void setGrouping(const QString& grouping);
    void setYear(const QString& year);
    void setTrackNumber(const QString& trackNumber);
    void setTrackTotal(const QString& trackTotal);
    void setComment(const QString& comment);
    void setKeyText(const QString& keyText);
    void setColor(const QColor& color);

  signals:
    void trackLoading();
    void trackLoaded();
    void trackUnloaded();
    void trackChanged();
    void cloneFromGroup(const QString& group);

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
    void commentChanged();
    void keyTextChanged();
    void colorChanged();
    void coverArtUrlChanged();
    void trackLocationUrlChanged();
    void cuesChanged();

    void loadTrackFromLocationRequested(const QString& trackLocation, bool play);

    void waveformLengthChanged();
    void waveformTextureChanged();
    void waveformTextureSizeChanged();
    void waveformTextureStrideChanged();

  private:
    QImage m_waveformTexture;
    QPointer<BaseTrackPlayer> m_pTrackPlayer;
    TrackPointer m_pCurrentTrack;
    QmlBeatsModel* m_pBeatsModel;
    QmlCuesModel* m_pHotcuesModel;
};

} // namespace qml
} // namespace mixxx
