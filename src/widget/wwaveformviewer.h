#ifndef WWAVEFORMVIEWER_H
#define WWAVEFORMVIEWER_H

#include <QDateTime>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QEvent>
#include <QList>
#include <QMutex>

#include "track/track.h"
#include "widget/trackdroptarget.h"
#include "widget/wwidget.h"
#include "skin/skincontext.h"

class ControlProxy;
class WaveformWidgetAbstract;
class ControlPotmeter;

class WWaveformViewer : public WWidget, public TrackDropTarget {
    Q_OBJECT
  public:
    WWaveformViewer(const char *group, UserSettingsPointer pConfig, QWidget *parent=nullptr);
    ~WWaveformViewer() override;

    const char* getGroup() const { return m_pGroup;}
    void setup(const QDomNode& node, const SkinContext& context);

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void mousePressEvent(QMouseEvent * /*unused*/) override;
    void mouseMoveEvent(QMouseEvent * /*unused*/) override;
    void mouseReleaseEvent(QMouseEvent * /*unused*/) override;

signals:
    void trackDropped(QString filename, QString group);
    void cloneDeck(QString source_group, QString target_group);

public slots:
    void slotTrackLoaded(TrackPointer track);
    void slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void onZoomChange(double zoom);
    void slotWidgetDead() {
        m_waveformWidget = nullptr;
    }

private:
    void setWaveformWidget(WaveformWidgetAbstract* waveformWidget);
    WaveformWidgetAbstract* getWaveformWidget() {
        return m_waveformWidget;
    }
    //direct access to let factory sync/set default zoom
    void setZoom(double zoom);
    void setDisplayBeatGridAlpha(int alpha);
    void setPlayMarkerPosition(double position);

private:
    const char* m_pGroup;
    UserSettingsPointer m_pConfig;
    int m_zoomZoneWidth;
    ControlProxy* m_pZoom;
    ControlProxy* m_pScratchPositionEnable;
    ControlProxy* m_pScratchPosition;
    ControlProxy* m_pWheel;
    bool m_bScratching;
    bool m_bBending;
    QPoint m_mouseAnchor;

    WaveformWidgetAbstract* m_waveformWidget;

    friend class WaveformWidgetFactory;
};

#endif
