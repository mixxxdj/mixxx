#pragma once

#include <QObject>
#include <QtDBus/QDBusObjectPath>

#include "broadcast/mpris/mpris.h"
#include "control/controlproxy.h"
#include "library/autodj/autodjprocessor.h"

class PlayerManager;
class MixxxMainWindow;

class MprisPlayer : public QObject {
    Q_OBJECT
  public:
    MprisPlayer(PlayerManager* pPlayerManager,
            MixxxMainWindow* pWindow,
            Mpris* pMpris);
    ~MprisPlayer() override;
    QString playbackStatus() const;
    QString loopStatus() const;
    void setLoopStatus(const QString& value);
    QVariantMap metadata() const;
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
    void playPause();
    void play();
    void stop();
    void seek(qlonglong offset);
    void setPosition(const QDBusObjectPath& trackId, qlonglong position);
    void openUri(const QString& uri);

  private slots:
    void mixxxComponentsInitialized();
    void slotChangeProperties(double enabled);
    void slotAutoDJIdle(double idle);
    void slotPlayChanged(DeckAttributes* pDeck, bool playing);
    void slotPlayPositionChanged(DeckAttributes* pDeck, double position);

  private:
    void broadcastPropertiesChange(bool enabled);
    QVariantMap getMetadataFromTrack(TrackPointer pTrack) const;
    DeckAttributes* findPlayingDeck() const;
    const QString autoDJDependentProperties[4] = {
            "CanGoNext",
            "CanPlay",
            "CanPause",
            "CanSeek"};

    ControlProxy* m_pCPAutoDjEnabled;
    ControlProxy* m_pCPFadeNow;
    ControlProxy* m_pCPAutoDJIdle;
    PlayerManager* m_pPlayerManager;
    MixxxMainWindow* m_pWindow;
    QString m_pausedDeck;
    QString m_thisDeck;
    TrackId m_currentMetadata;
    bool m_bComponentsInitialized, m_bPropertiesEnabled;
    Mpris* m_pMpris;
    QList<DeckAttributes*> m_deckAttributes;
};
