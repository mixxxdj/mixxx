
#ifndef WWAVEFORMVIEWER_H
#define WWAVEFORMVIEWER_H

#include <QList>
#include <QEvent>
#include <QDateTime>
#include <QMutex>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QTimerEvent>

#include "trackinfoobject.h"
#include "wwidget.h"
#include "defs.h"

class EngineBuffer;
class WaveformWidgetAbstract;

class WWaveformViewer : public QWidget
{
    Q_OBJECT

public:
    WWaveformViewer(const char *group, QWidget *parent=0, Qt::WFlags f = 0);
    virtual ~WWaveformViewer();

    const char* getGroup() const { return m_pGroup;}

    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void setup(QDomNode node = QDomNode());

    bool eventFilter(QObject *o, QEvent *e);

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
    void setWaveformWidget( WaveformWidgetAbstract* waveformWidget) { m_waveformWidget = waveformWidget;}
    WaveformWidgetAbstract* getWaveformWidget() { return m_waveformWidget;}

private:
    const char *m_pGroup;
    int m_zoomZoneWidth;
    int m_iMouseStart;

    WaveformWidgetAbstract* m_waveformWidget;

    friend class WaveformWidgetFactory;
};

#endif
