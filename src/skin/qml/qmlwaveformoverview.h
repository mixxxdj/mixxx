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
    Q_PROPERTY(QColor colorHigh MEMBER m_colorHigh NOTIFY colorHighChanged)
    Q_PROPERTY(QColor colorMid MEMBER m_colorMid NOTIFY colorMidChanged)
    Q_PROPERTY(QColor colorLow MEMBER m_colorLow NOTIFY colorLowChanged)

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
    void colorHighChanged(const QColor& color);
    void colorMidChanged(const QColor& color);
    void colorLowChanged(const QColor& color);

  private:
    void setCurrentTrack(TrackPointer pTrack);
    QColor getPenColor(ConstWaveformPointer pWaveform, int completion) const;

    QmlPlayerProxy* m_pPlayer;
    TrackPointer m_pCurrentTrack;
    QColor m_colorHigh;
    QColor m_colorMid;
    QColor m_colorLow;
};

} // namespace qml
} // namespace skin
} // namespace mixxx
