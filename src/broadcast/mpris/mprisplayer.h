#pragma once

#include <QFuture>
#include <QLinkedList>
#include <QObject>
#include <QTemporaryFile>
#include <QtDBus/QDBusObjectPath>

#include "broadcast/mpris/mpris.h"
#include "control/controlproxy.h"
#include "library/autodj/autodjprocessor.h"
#include "mixxxmainwindow.h"

class PlayerManager;
class MixxxMainWindow;
class PlayerManagerInterface;

class MprisPlayer : public QObject {
    Q_OBJECT
  public:
    MprisPlayer(PlayerManagerInterface* pPlayerManager,
            Mpris* pMpris,
            UserSettingsPointer pSettings);

    QString playbackStatus() const;
    QString loopStatus() const;
    void setLoopStatus(const QString& value);
    double rate() const;
    void setRate(double value);
    QVariantMap metadata();
    double volume() const;
    void setVolume(double value);
    qlonglong position() const;
    bool canGoNext() const;
    bool canGoPrevious() const;
    bool canPlay() const;
    bool canPause() const;
    bool canSeek() const;
    void nextTrack();
    void pause();
    void stop();
    void playPause();
    void play();
    qlonglong seek(qlonglong offset, bool& success);
    qlonglong setPosition(const QDBusObjectPath& trackId, qlonglong position, bool& success);
    void openUri(const QString& uri);

  private slots:
    void slotChangeProperties(double enabled);
    void slotPlayChanged(DeckAttributes* pDeck, bool playing);
    void slotPlayPositionChanged(DeckAttributes* pDeck, double position);
    void slotMasterGainChanged(double volume);
    void slotCoverArtFound(const QObject* requester,
            const CoverInfoRelative& info,
            const QPixmap& pixmap);
    void slotPlayingTrackChanged(TrackPointer pTrack);

  private:
    void broadcastPropertiesChange(bool enabled);
    QFuture<void> requestMetadataFromTrack(TrackPointer pTrack, bool requestCover);
    QFuture<void> requestCoverArtUrl(TrackPointer pTrack);
    void broadcastCurrentMetadata();
    QVariantMap getVariantMapMetadata();
    DeckAttributes* findPlayingDeck() const;
    bool autoDjEnabled() const;
    const QString autoDJDependentProperties[4] = {
            "CanGoNext",
            "CanPlay",
            "CanPause",
            "CanSeek"};

    ControlProxy m_pCPAutoDjEnabled;
    ControlProxy m_pCPFadeNow;
    ControlProxy m_pCPMasterGain;
    PlayerManagerInterface* m_pPlayerManager;
    bool m_bPropertiesEnabled;
    Mpris* m_pMpris;
    std::vector<std::unique_ptr<DeckAttributes>> m_deckAttributes;
    DeckAttributes* m_pPlayableDeck;
    UserSettingsPointer m_pSettings;

    class CurrentMetadata {
      public:
        CurrentMetadata();
        void idle(bool autoDjEnabled);
        void newCoverArt();
        QString coverArtUrl() const;

        QString trackPath;
        double trackDuration;
        QStringList artists;
        QString title;
        QUrl url;
        QString album;
        int userRating;
        int useCount;
        std::unique_ptr<QTemporaryFile> pCoverArtFile;

      private:
        QTemporaryFile defaultCoverArtFile;
    };

    CurrentMetadata m_currentMetadata;
};
