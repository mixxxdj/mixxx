
#ifndef WGLWAVEFORMVIEWER_H
#define WGLWAVEFORMVIEWER_H

#include <qgl.h>
#include <QList>
#include <QEvent>
#include <QDateTime>
#include <QMutex>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QTimerEvent>
#include <QGLContext>
#include <QtDebug>

#include "wwidget.h"
#include "defs.h"

class EngineBuffer;
class WaveformRenderer;
class ControlObjectThreadMain;

class WGLWaveformViewer : public QGLWidget
{
    Q_OBJECT
  public:
    WGLWaveformViewer(const char *group, WaveformRenderer* pWaveformRenderer,
                      QWidget *pParent=0, const QGLWidget *pShareWidget = 0,
                      QGLContext *ctxt = 0,
                      Qt::WFlags f = 0);
    virtual ~WGLWaveformViewer();

    bool directRendering();
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void setup(QDomNode node);
    bool eventFilter(QObject *o, QEvent *e);

  public slots:
    void setValue(double);
    void refresh();

  signals:
    void valueChangedLeftDown(double);
    void valueChangedRightDown(double);
    void trackDropped(QString filename, QString group);

  protected:
    void paintEvent(QPaintEvent* event);
    void resizeEvent(QResizeEvent* e);

  private:
    /** Used in mouse event handler */
    int m_iMouseStart;

    /** Waveform Renderer does all the work for us */
    WaveformRenderer *m_pWaveformRenderer;

    ControlObjectThreadMain* m_pScratchEnable;
    ControlObjectThreadMain* m_pScratch;
    ControlObjectThreadMain* m_pTrackSamples;
    ControlObjectThreadMain* m_pTrackSampleRate;
    ControlObjectThreadMain* m_pRate;
    ControlObjectThreadMain* m_pRateRange;
    ControlObjectThreadMain* m_pRateDir;

    bool m_bScratching;
    bool m_painting;
    QMutex m_paintMutex;

    const char *m_pGroup;
};

#endif
