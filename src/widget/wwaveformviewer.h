#ifndef WWAVEFORMVIEWER_H
#define WWAVEFORMVIEWER_H

#include <QDateTime>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QEvent>
#include <QList>
#include <QMutex>

#include "trackinfoobject.h"
#include "widget/wwidget.h"
#include "skin/skincontext.h"

class ControlObjectSlave;
class WaveformWidgetAbstract;
class ControlPotmeter;

class WWaveformViewer : public WWidget {
    Q_OBJECT
  public:
    WWaveformViewer(const char *group, ConfigObject<ConfigValue>* pConfig, QWidget *parent=0);
    virtual ~WWaveformViewer();

    const char* getGroup() const { return m_pGroup;}
    void setup(QDomNode node, const SkinContext& context);

    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

signals:
    void trackDropped(QString filename, QString group);

public slots:
    void onTrackLoaded(TrackPointer track);
    void onTrackUnloaded(TrackPointer track);

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void wheelEvent(QWheelEvent *event);

private slots:
    void onZoomChange(double zoom);
    void slotWidgetDead() {
        m_waveformWidget = NULL;
    }

private:
    void setWaveformWidget(WaveformWidgetAbstract* waveformWidget);
    WaveformWidgetAbstract* getWaveformWidget() {
        return m_waveformWidget;
    }
    //direct access to let factory sync/set default zoom
    void setZoom(int zoom);

private:
    const char* m_pGroup;
    ConfigObject<ConfigValue>* m_pConfig;
    int m_zoomZoneWidth;
    ControlObjectSlave* m_pZoom;
    ControlObjectSlave* m_pScratchPositionEnable;
    ControlObjectSlave* m_pScratchPosition;
    ControlObjectSlave* m_pWheel;
    bool m_bScratching;
    bool m_bBending;
    QPoint m_mouseAnchor;

    WaveformWidgetAbstract* m_waveformWidget;

    friend class WaveformWidgetFactory;
};

#endif
