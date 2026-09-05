#pragma once
#include <QColor>
#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QString>
#include <QUrl>

#include "analyzer/analyzerprogress.h"
#include "mixer/basetrackplayer.h"
#include "qmltrackproxy.h"
#include "track/track_decl.h"

namespace mixxx {
namespace qml {

class QmlPlayerProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(QmlTrackProxy* currentTrack READ currentTrack NOTIFY trackChanged)
    Q_PROPERTY(double analyzerProgress READ analyzerProgress NOTIFY analyzerProgressChanged)
    Q_PROPERTY(bool isLoaded READ isLoaded NOTIFY trackChanged)
    Q_PROPERTY(bool trackLoaded READ isTrackLoaded NOTIFY trackLoadedChanged)
    QML_NAMED_ELEMENT(Player)
    QML_UNCREATABLE("Only accessible via Mixxx.PlayerManager.getPlayer(group)")

  public:
    explicit QmlPlayerProxy(BaseTrackPlayer* pTrackPlayer, QObject* parent = nullptr);

    bool isLoaded() const;
    bool isTrackLoaded() const;
    double analyzerProgress() const;
    /// Needed for interacting with the raw track player object.
    BaseTrackPlayer* internalTrackPlayer() const {
        return m_pTrackPlayer;
    }

    Q_INVOKABLE void loadTrack(mixxx::qml::QmlTrackProxy* track, bool play = false);
    Q_INVOKABLE void loadTrackFromLocation(const QString& trackLocation, bool play = false);
    Q_INVOKABLE void loadTrackFromLocationUrl(const QUrl& trackLocationUrl, bool play = false);

  public slots:
    void slotTrackLoaded(TrackPointer pTrack);
    void slotTrackUnloaded(TrackPointer pOldTrack);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void slotTrackAnalyzerProgress(TrackId trackId, AnalyzerProgress analyzerProgress);

  signals:
    void trackLoading();
    void trackLoaded();
    void trackUnloaded();
    void trackChanged();
    void trackLoadedChanged();
    void analyzerProgressChanged();
    void cloneFromGroup(const QString& group);

    void loadTrackFromLocationRequested(const QString& trackLocation, bool play);
    void loadTrackRequested(TrackPointer track,
#ifdef __STEM__
            mixxx::StemChannelSelection stemSelection,
#endif
            bool play);

  private:
    QmlTrackProxy* currentTrack();
    void setAnalyzerProgress(AnalyzerProgress analyzerProgress);

    QPointer<BaseTrackPlayer> m_pTrackPlayer;
    TrackPointer m_pCurrentTrack;
    AnalyzerProgress m_analyzerProgress{kAnalyzerProgressUnknown};
    bool m_trackLoaded{false};
};

} // namespace qml
} // namespace mixxx
