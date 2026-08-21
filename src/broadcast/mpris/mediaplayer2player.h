#pragma once

#include <QStringList>
#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusObjectPath>

#include "broadcast/mpris/mprisplayer.h"

class AutoDJProcessor;
class PlayerManager;
class MixxxMainWindow;

// this implements the Version 2.2 of
// MPRIS D-Bus Interface Specification
// org.mpris.MediaPlayer2.Player
// http://specifications.freedesktop.org/mpris-spec/2.2/

class MediaPlayer2Player : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")
    Q_PROPERTY(QString PlaybackStatus READ playbackStatus)
    Q_PROPERTY(QString LoopStatus READ loopStatus WRITE setLoopStatus)
    Q_PROPERTY(double Rate READ rate WRITE setRate)
    Q_PROPERTY(bool Shuffle READ shuffle WRITE setShuffle) // optional
    Q_PROPERTY(QVariantMap Metadata READ metadata)
    Q_PROPERTY(double Volume READ volume WRITE setVolume)
    Q_PROPERTY(qlonglong Position READ position)
    Q_PROPERTY(double MinimumRate READ minimumRate)
    Q_PROPERTY(double MaximumRate READ maximumRate)
    Q_PROPERTY(bool CanGoNext READ canGoNext)
    Q_PROPERTY(bool CanGoPrevious READ canGoPrevious)
    Q_PROPERTY(bool CanPlay READ canPlay)
    Q_PROPERTY(bool CanPause READ canPause)
    Q_PROPERTY(bool CanSeek READ canSeek)
    Q_PROPERTY(bool CanControl READ canControl)

  public:
    explicit MediaPlayer2Player(PlayerManagerInterface* playerManager,
            Mpris* pMpris,
            UserSettingsPointer pSettings);

    ~MediaPlayer2Player() override = default;

    QString playbackStatus() const;
    QString loopStatus() const;
    void setLoopStatus(const QString& value);
    double rate() const;
    void setRate(double value);
    bool shuffle() const;
    void setShuffle(bool value);
    QVariantMap metadata();
    double volume() const;
    void setVolume(double value);
    qlonglong position() const;
    double minimumRate() const;
    double maximumRate() const;
    bool canGoNext() const;
    bool canGoPrevious() const;
    bool canPlay() const;
    bool canPause() const;
    bool canSeek() const;
    bool canControl() const;

  public slots:
    void Next();
    void Previous();
    void Pause();
    void PlayPause();
    void Stop();
    void Play();
    void Seek(qlonglong offset);
    void SetPosition(const QDBusObjectPath& trackId, qlonglong position);
    void OpenUri(const QString& uri);

  signals:
    void Seeked(qlonglong position);

  private:
    MprisPlayer m_mprisPlayer;
};
