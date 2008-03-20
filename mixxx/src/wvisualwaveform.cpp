/***************************************************************************
                          wvisualwaveform.cpp  -
                             -------------------
    begin                : Thu Oct 9 2003
    copyright            : (C) 2002 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QDropEvent>
#include <QTimerEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QDragEnterEvent>
#include <QUrl>

#include "wvisualwaveform.h"
#include "wskincolor.h"
#include "visual/visualchannel.h"
#include "visual/visualdisplay.h"

WVisualWaveform::WVisualWaveform(QWidget * pParent, const QGLWidget * pShareWidget) : QGLWidget(pParent, pShareWidget)
{
    setAcceptDrops(true);
    m_pVisualController = new VisualController();

    installEventFilter(this);

#ifdef __MACX__
    // Hack to reduce load in GUI thread. This makes the system behave
    // "correctly" on MacOS X, where it would otherwise stall the system
    // for some seconds now and then.
    //m_iTimerID = startTimer(100);

    //The above hack makes Mixxx feel like it's running on a 386 on OS X Intel.
    //I'm going to experiment a bit with the timings - Albert:
    m_iTimerID = startTimer(30);
#endif
#ifdef __WIN__
    m_iTimerID = startTimer(30);
#endif
#ifdef __LINUX__
    m_iTimerID = startTimer(30);
#endif

    m_painting = false;
}

WVisualWaveform::~WVisualWaveform()
{
    // Stop timer
    killTimer(m_iTimerID);

    // Delete associated VisualChannels
    //while (!m_qlList.isEmpty())
    //    delete m_qlList.takeFirst();

    // Finally delete the VisualController
    delete m_pVisualController;
}

bool WVisualWaveform::directRendering()
{
    return format().directRendering();
}

void WVisualWaveform::dragEnterEvent(QDragEnterEvent * event)
{
  if (event->mimeData()->hasUrls())
      event->acceptProposedAction();
}

void WVisualWaveform::dropEvent(QDropEvent * event)
{
  if (event->mimeData()->hasUrls()) {
    QList<QUrl> urls(event->mimeData()->urls());
    QUrl url = urls.first();
    QString name = url.path();

    event->accept();
    emit(trackDropped(name));
  } else
    event->ignore();
}

void WVisualWaveform::setup(QDomNode node)
{
    // Colors
    colorBack.setNamedColor(WWidget::selectNodeQString(node, "BgColor"));
    colorBack = WSkinColor::getCorrectColor(colorBack);
    m_pVisualController->setBackgroundColor(colorBack);
    colorSignal.setNamedColor(WWidget::selectNodeQString(node, "SignalColor"));
    colorSignal = WSkinColor::getCorrectColor(colorSignal);
    colorHfc.setNamedColor(WWidget::selectNodeQString(node, "HfcColor"));
    colorHfc = WSkinColor::getCorrectColor(colorHfc);
    colorCue.setNamedColor(WWidget::selectNodeQString(node, "CueColor"));
    colorCue = WSkinColor::getCorrectColor(colorCue);
    colorMarker.setNamedColor(WWidget::selectNodeQString(node, "MarkerColor"));
    colorMarker = WSkinColor::getCorrectColor(colorMarker);
    colorBeat.setNamedColor(WWidget::selectNodeQString(node, "BeatColor"));
    colorBeat = WSkinColor::getCorrectColor(colorBeat);
    colorFisheye.setNamedColor(WWidget::selectNodeQString(node, "FisheyeColor"));
    colorFisheye = WSkinColor::getCorrectColor(colorFisheye);

    // Set position
    QString pos = WWidget::selectNodeQString(node, "Pos");
    int x = pos.left(pos.indexOf(",")).toInt();
    int y = pos.mid(pos.indexOf(",")+1).toInt();
    move(x,y);

    // Size
    QString size = WWidget::selectNodeQString(node, "Size");
    x = size.left(size.indexOf(",")).toInt();
    y = size.mid(size.indexOf(",")+1).toInt();
    setFixedSize(x,y);
}

bool WVisualWaveform::eventFilter(QObject * o, QEvent * e)
{
    // Handle mouse press events
    if (e->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent * m = (QMouseEvent *)e;

        m_iStartPosX = -1;
        if (m->button()==Qt::LeftButton)
        {
            // Store current x position of mouse pointer
            m_iStartPosX = m->x();
            emit(valueChangedLeftDown(64.));
        }
/*
        else if (m->button()==Qt::RightButton)
        {
            // Toggle fish eye mode on each channel associated
            VisualChannel *c;
            for (c=m_qlList.first(); c; c=m_qlList.next())
                c->toggleFishEyeMode();
        }
 */
    }
    else if (e->type() == QEvent::MouseMove)
    {
        // Only process mouse move if it was initiated by a left click
        if (m_iStartPosX!=-1)
        {
            QMouseEvent * m = (QMouseEvent *)e;
            double v = 64.+(double)(m->x()-m_iStartPosX)/10.;
            if (v<0.)
                v = 0.;
            else if (v>127.)
                v= 127.;
            emit(valueChangedLeftDown(v));
        }
    }
    else if (e->type() == QEvent::MouseButtonRelease)
    {
        emit(valueChangedLeftDown(64.));
    }
    else
    {
        // standard event processing
        return QObject::eventFilter(o,e);
    }
    return true;
}

void WVisualWaveform::slotNewTrack()
{
    // Call each channel associated
    VisualChannel * c;
    for (int i = 0; i < m_qlList.size(); ++i) {
        c = m_qlList[i];
        c->setupBuffer();
    }
}

VisualChannel * WVisualWaveform::add(const char * group)
{
    VisualChannel * c = new VisualChannel(m_pVisualController, group);

    // Position coding... hack
    //if (m_qlList.isEmpty())
    {
/*
        c->setPosX(-(width()/2));
        c->setLength(800); //width());
        c->setHeight(50); //height());
        c->setZoomPosX(50);
 */
        c->setColorBack((float)colorBack.red()/255., (float)colorBack.green()/255., (float)colorBack.blue()/255.);
        c->setColorSignal((float)colorSignal.red()/255., (float)colorSignal.green()/255., (float)colorSignal.blue()/255.);
        c->setColorHfc((float)colorHfc.red()/255., (float)colorHfc.green()/255., (float)colorHfc.blue()/255.);
        c->setColorCue((float)colorCue.red()/255., (float)colorCue.green()/255., (float)colorCue.blue()/255.);
        c->setColorMarker((float)colorMarker.red()/255., (float)colorMarker.green()/255., (float)colorMarker.blue()/255.);
        c->setColorBeat((float)colorBeat.red()/255., (float)colorBeat.green()/255., (float)colorBeat.blue()/255.);
        c->setColorFisheye((float)colorFisheye.red()/255., (float)colorFisheye.green()/255., (float)colorFisheye.blue()/255.);
    }
    /* else
       {
        c->setPosX(50);
        c->setZoomPosX(50);
       } */

    m_qlList.append(c);
    return c;
}

void WVisualWaveform::initializeGL()
{
    m_pVisualController->init();
    //m_pVisualBackplane = new VisualBackplane();
//    controller->add(m_pVisualBackplane);

    m_Picking.init(m_pVisualController);
}


void WVisualWaveform::paintGL()
{
    // Display stuff
    makeCurrent();
    m_pVisualController->display();
    m_painting = false;
}

void WVisualWaveform::resizeGL(int width, int height)
{
    m_pVisualController->resize((GLsizei)width,(GLsizei)height);
}

void WVisualWaveform::timerEvent(QTimerEvent *)
{
    // FIXME: This should be done with a QMutex deal
    // Update: I added the mutex, but I'm not totally sure
    // of the consequences of having it, so I'm leaving it
    // out for now. Someone should look at this closer.
    // - Albert Aug 19/07
    m_paintMutex.lock();
    if (!m_painting) {
        m_painting = true;
        updateGL();
    }
    m_paintMutex.unlock();
}

void WVisualWaveform::resetColors() {
    VisualChannel * vc;

    for (int i = 0; i < m_qlList.size(); ++i) {
        vc = m_qlList[i];
        vc->setColorBack((float)colorBack.red()/255., (float)colorBack.green()/255., (float)colorBack.blue()/255.);
        vc->setColorSignal((float)colorSignal.red()/255., (float)colorSignal.green()/255., (float)colorSignal.blue()/255.);
        vc->setColorHfc((float)colorHfc.red()/255., (float)colorHfc.green()/255., (float)colorHfc.blue()/255.);
        vc->setColorCue((float)colorCue.red()/255., (float)colorCue.green()/255., (float)colorCue.blue()/255.);
        vc->setColorMarker((float)colorMarker.red()/255., (float)colorMarker.green()/255., (float)colorMarker.blue()/255.);
        vc->setColorBeat((float)colorBeat.red()/255., (float)colorBeat.green()/255., (float)colorBeat.blue()/255.);
        vc->setColorFisheye((float)colorFisheye.red()/255., (float)colorFisheye.green()/255., (float)colorFisheye.blue()/255.);
        vc->resetColors();
    }
}

