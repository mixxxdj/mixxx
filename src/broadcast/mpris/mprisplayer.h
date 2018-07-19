#pragma once

#include <QObject>
#include <QtDBus/QDBusObjectPath>

#include "control/controlproxy.h"
#include "mpris.h"

class PlayerManager;
class MixxxMainWindow;

class MprisPlayer : public QObject {
    Q_OBJECT
  public:
    MprisPlayer(PlayerManager* pPlayerManager,
            MixxxMainWindow* pWindow,
            Mpris* pMpris);
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
    void autoDJStateChanged(double enabled);

  private:
    void broadcastPropertiesChange(bool enabled);

    const QString autoDJDependentProperties[4] = {"CanGoNext",
            "CanPlay",
            "CanPause",
            "CanSeek"};
    ControlProxy* m_pCPAutoDjEnabled;
    PlayerManager* m_pPlayerManager;
    MixxxMainWindow* m_pWindow;
    bool m_bComponentsInitialized;
    Mpris* m_pMpris;
};
