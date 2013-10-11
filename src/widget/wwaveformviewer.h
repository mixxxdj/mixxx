#ifndef WWAVEFORMVIEWER_H
#define WWAVEFORMVIEWER_H

#include <QDateTime>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QEvent>
#include <QList>
#include <QMutex>

#include "defs.h"
#include "trackinfoobject.h"
#include "widget/wwidget.h"

class ControlObjectThread;
class WaveformWidgetAbstract;
class ControlPotmeter;

class WWaveformViewer : public QWidget {
    Q_OBJECT
  public:
    WWaveformViewer(const char *group, ConfigObject<ConfigValue>* pConfig, QWidget *parent=0);
    virtual ~WWaveformViewer();

    const char* getGroup() const { return m_pGroup;}
    void setup(QDomNode node = QDomNode());

    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

signals:
    void valueChangedLeftDown(double);
    void valueChangedRightDown(double);
    void trackDropped(QString filename, QString group);
    void valueReset();

public slots:
    void onTrackLoaded( TrackPointer track);
    void onTrackUnloaded( TrackPointer track);

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
    ControlObjectThread* m_pZoom;
    ControlObjectThread* m_pScratchPositionEnable;
    ControlObjectThread* m_pScratchPosition;
    bool m_bScratching;
    bool m_bBending;
    QPoint m_mouseAnchor;

    WaveformWidgetAbstract* m_waveformWidget;

    friend class WaveformWidgetFactory;
};

#endif
