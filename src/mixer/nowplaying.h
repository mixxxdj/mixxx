#pragma once

#include <QList>
#include <QObject>
#include <QString>
#include <QThread>
#include <QTimer>
#include <memory>

#include "preferences/usersettings.h"
#include "track/track.h"

class ControlProxy;

class NowPlayingWorker : public QObject {
    Q_OBJECT

  public:
    explicit NowPlayingWorker(UserSettingsPointer pConfig, QObject* parent = nullptr);
    ~NowPlayingWorker() override;

    void initialize();
    void shutdown();

  signals:
    void updateWindowTitle(const QString& title, const QString& filePath);
    void fileWriteRequested(const QString& content);

  private slots:
    void slotPollNowPlaying();
    void slotTrackChanged(const QString& group, TrackPointer pTrack, TrackPointer pOldTrack);

  private:
    struct DeckState {
        QString group;
        TrackPointer currentTrack;
        double volume;
        double pregain;
        double crossfaderOrientation;
        bool isPlaying;
        double effectiveVolume;

        void calculateEffectiveVolume(double crossfaderValue);
    };

    UserSettingsPointer m_pConfig;

    QList<ControlProxy*> m_deckPlayProxies;
    QList<ControlProxy*> m_deckPregainProxies;
    QList<ControlProxy*> m_deckVolumeProxies;
    QList<ControlProxy*> m_deckCrossfaderOrientationProxies;
    ControlProxy* m_pMasterCrossfaderProxy;

    QList<DeckState> m_deckStates;
    QTimer* m_pollTimer;
    QString m_nowPlayingFilePath;
    QString m_archiveFolderPath;
    bool m_sessionArchived;

    bool m_nowPlayingEnabled;
    int m_pollIntervalMs;
    bool m_appendMode;
    bool m_addTimestamp;
    bool m_archive;

    void writeNowPlayingFile(const QString& content);
    int getDeckIndexFromGroup(const QString& group) const;
    void archiveCurrentSession();
    void ensureArchiveFolderExists();
    void writeSessionHeader();
};

class NowPlaying : public QObject {
    Q_OBJECT

  public:
    explicit NowPlaying(UserSettingsPointer pConfig, QObject* parent = nullptr);
    ~NowPlaying() override;

    void initialize();
    void shutdown();

  private slots:
    void slotUpdateWindowTitle(const QString& title, const QString& filePath);

  private:
    UserSettingsPointer m_pConfig;
    QThread m_workerThread;
    NowPlayingWorker* m_pWorker;
};

// #pragma once
//
// #include <QList>
// #include <QObject>
// #include <QString>
// #include <QTimer>
// #include <memory>
//
// #include "preferences/usersettings.h"
// #include "track/track.h"
//
// class ControlProxy;
//
// class NowPlaying : public QObject {
//     Q_OBJECT
//
//   public:
//     NowPlaying(UserSettingsPointer pConfig,
//             QObject* parent = nullptr);
//     ~NowPlaying();
//
//     void initialize();
//     void shutdown();
//
//   private slots:
//     void slotPollNowPlaying();
//     void slotTrackChanged(const QString& group, TrackPointer pTrack, TrackPointer pOldTrack);
//
//   private:
//     struct DeckState {
//         QString group;
//         TrackPointer currentTrack;
//         double volume;
//         double pregain;
//         double crossfaderOrientation;
//         bool isPlaying;
//         double effectiveVolume;
//
//         void calculateEffectiveVolume(double crossfaderValue);
//     };
//
//     UserSettingsPointer m_pConfig;
//
//     QList<ControlProxy*> m_deckPlayProxies;
//     QList<ControlProxy*> m_deckPregainProxies;
//     QList<ControlProxy*> m_deckVolumeProxies;
//     QList<ControlProxy*> m_deckCrossfaderOrientationProxies;
//     ControlProxy* m_pMasterCrossfaderProxy;
//
//     QList<DeckState> m_deckStates;
//     QTimer m_pollTimer;
//     QString m_nowPlayingFilePath;
//     QString m_archiveFolderPath;
//     bool m_sessionArchived;
//
//     bool m_nowPlayingEnabled;
//     int m_pollIntervalMs;
//     bool m_appendMode;
//     bool m_addTimestamp;
//     bool m_archive;
//
//     void writeNowPlayingFile(const QString& content);
//     void updateWindowTitle(const QList<DeckState>& audibleDecks);
//     int getDeckIndexFromGroup(const QString& group);
//     void archiveCurrentSession();
//     void ensureArchiveFolderExists();
//     void writeSessionHeader();
// };
