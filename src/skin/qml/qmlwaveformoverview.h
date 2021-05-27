#pragma once

#include <QPainter>
#include <QQuickItem>
#include <QQuickPaintedItem>
#include <QtQml>

#include "track/track.h"

namespace mixxx {
namespace skin {
namespace qml {

class QmlPlayerProxy;

class QmlWaveformOverview : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(mixxx::skin::qml::QmlPlayerProxy* player READ getPlayer
                    WRITE setPlayer NOTIFY playerChanged)

  public:
    QmlWaveformOverview(QQuickItem* parent = nullptr);
    void paint(QPainter* painter);

    void setPlayer(QmlPlayerProxy* player);
    QmlPlayerProxy* getPlayer() const;

  private slots:
    void slotTrackLoaded(TrackPointer pLoadedTrack);
    void slotTrackLoading(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void slotTrackUnloaded();
    void slotWaveformUpdated();

  signals:
    void playerChanged();

  private:
    void setCurrentTrack(TrackPointer pTrack);

    QmlPlayerProxy* m_pPlayer;
    TrackPointer m_pCurrentTrack;
};

} // namespace qml
} // namespace skin
} // namespace mixxx
