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

class ControlObjectThreadMain;
class WaveformWidgetAbstract;

class WWaveformViewer : public QWidget {
    Q_OBJECT

  public:
    WWaveformViewer(const char *group, ConfigObject<ConfigValue>* pConfig, QWidget *parent=0, Qt::WFlags f = 0);
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

  public slots:
    void onTrackLoaded( TrackPointer track);
    void onTrackUnloaded( TrackPointer track);

  protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void wheelEvent(QWheelEvent *event);

  private:
    void setWaveformWidget(WaveformWidgetAbstract* waveformWidget) {
        m_waveformWidget = waveformWidget;
    }
    WaveformWidgetAbstract* getWaveformWidget() {
        return m_waveformWidget;
    }

    const char* m_pGroup;
    ConfigObject<ConfigValue>* m_pConfig;
    int m_zoomZoneWidth;

    ControlObjectThreadMain* m_pScratchEnable;
    ControlObjectThreadMain* m_pScratch;
    ControlObjectThreadMain* m_pTrackSamples;
    ControlObjectThreadMain* m_pTrackSampleRate;
    ControlObjectThreadMain* m_pRate;
    ControlObjectThreadMain* m_pRateRange;
    ControlObjectThreadMain* m_pRateDir;
    bool m_bScratching;
    bool m_bBending;
    int m_iMouseStart;
    QPoint m_mouseAnchor;

    WaveformWidgetAbstract* m_waveformWidget;

    friend class WaveformWidgetFactory;
};

#endif
