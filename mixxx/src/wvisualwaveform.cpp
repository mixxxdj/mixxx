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

#include "wvisualwaveform.h"
#include "visual/visualchannel.h"
#include "visual/visualdisplay.h"
#include <qdragobject.h>

WVisualWaveform::WVisualWaveform(QWidget *pParent, const char *pName, const QGLWidget *pShareWidget) : QGLWidget(pParent,pName,pShareWidget)
{
    setAcceptDrops(true);
    m_pVisualController = new VisualController();

    installEventFilter(this);
    m_qtTime.start();
#ifdef __MACX__
    // Hack to reduce load in GUI thread. This makes the system behave 
    // "correctly" on MacOS X, where it would otherwise stall the system
    // for some seconds now and then.
    startTimer(100);
#endif
#ifdef __WIN__
    startTimer(25);
#endif
#ifdef __LINUX__
    startTimer(25);
#endif

    m_qlList.setAutoDelete(false);
}

WVisualWaveform::~WVisualWaveform()
{
    // Stop timer
    killTimers();

    // Delete associated VisualChannels
    while (m_qlList.remove());

    // Finally delete the VisualController
    delete m_pVisualController;
}

bool WVisualWaveform::directRendering()
{
    return format().directRendering();
}

void WVisualWaveform::dragEnterEvent(QDragEnterEvent *event)
{
    event->accept(QUriDrag::canDecode(event));
}

void WVisualWaveform::dropEvent(QDropEvent *event)
{
    QStringList lst;
    if (!QUriDrag::canDecode(event))
    {
        event->ignore();
        return;
    }

    event->accept();
    QUriDrag::decodeLocalFiles(event, lst);
    QString name = (*lst.begin());

    emit(trackDropped(name));
}

void WVisualWaveform::setup(QDomNode node)
{
    // Colors
    colorBack.setNamedColor(WWidget::selectNodeQString(node, "BgColor"));
    m_pVisualController->setBackgroundColor(colorBack);
    colorSignal.setNamedColor(WWidget::selectNodeQString(node, "SignalColor"));
    colorHfc.setNamedColor(WWidget::selectNodeQString(node, "HfcColor"));
    colorMarker.setNamedColor(WWidget::selectNodeQString(node, "MarkerColor"));
    colorBeat.setNamedColor(WWidget::selectNodeQString(node, "BeatColor"));
    colorFisheye.setNamedColor(WWidget::selectNodeQString(node, "FisheyeColor"));

    // Set position
    QString pos = WWidget::selectNodeQString(node, "Pos");
    int x = pos.left(pos.find(",")).toInt();
    int y = pos.mid(pos.find(",")+1).toInt();
    move(x,y);

    // Size
    QString size = WWidget::selectNodeQString(node, "Size");
    x = size.left(size.find(",")).toInt();
    y = size.mid(size.find(",")+1).toInt();
    setFixedSize(x,y);


}

bool WVisualWaveform::eventFilter(QObject *o, QEvent *e)
{
    // Handle mouse press events
    if (e->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *m = (QMouseEvent *)e;

        m_iStartPosX = -1;
        if (m->button()==Qt::LeftButton)
        {
            // Store current x position of mouse pointer
            m_iStartPosX = m->x();
            emit(valueChangedLeftDown(64.));
        }
        else if (m->button()==Qt::RightButton)
        {
            // Toggle fish eye mode on each channel associated
            VisualChannel *c;
            for (c=m_qlList.first(); c; c=m_qlList.next())
                c->toggleFishEyeMode();
        }
    }
    else if (e->type() == QEvent::MouseMove)
    {
        // Only process mouse move if it was initiated by a left click
        if (m_iStartPosX!=-1)
        {
            QMouseEvent *m = (QMouseEvent *)e;
            int v = 64+m->x()-m_iStartPosX;
            if (v<0)
                v = 0;
            else if (v>127)
                v= 127;
            emit(valueChangedLeftDown((double)v));
        }
    }
    else if (e->type() == QEvent::MouseButtonRelease)
    {
        emit(valueChangedLeftDown(64.));
    }

/*
        int id = m_Picking.pick(m->x(),m->y());
        qDebug("pick id %i",id);
        
        VisualChannel *c;
        for (c = m_qlList.first(); c; c = m_qlList.next())
            c->zoom(id);
    }
*/

    else
    {
        // standard event processing
        return QObject::eventFilter(o,e);
    }
    return true;
}

VisualChannel *WVisualWaveform::add(ControlPotmeter *pPlaypos, const char *group)
{
    VisualChannel *c = new VisualChannel(pPlaypos, m_pVisualController, group);

    // Position coding... hack
    if (m_qlList.isEmpty())
    {
        c->setPosX(-(width()/2));
        c->setLength(width());
        c->setHeight(height());
        c->setZoomPosX(50);
        c->setColorBack((float)colorBack.red()/255., (float)colorBack.green()/255., (float)colorBack.blue()/255.);
        c->setColorSignal((float)colorSignal.red()/255., (float)colorSignal.green()/255., (float)colorSignal.blue()/255.);
        c->setColorHfc((float)colorHfc.red()/255., (float)colorHfc.green()/255., (float)colorHfc.blue()/255.);
        c->setColorMarker((float)colorMarker.red()/255., (float)colorMarker.green()/255., (float)colorMarker.blue()/255.);
        c->setColorBeat((float)colorBeat.red()/255., (float)colorBeat.green()/255., (float)colorBeat.blue()/255.);
        c->setColorFisheye((float)colorFisheye.red()/255., (float)colorFisheye.green()/255., (float)colorFisheye.blue()/255.);
    }
    else
    {
        c->setPosX(50);
        c->setZoomPosX(50);
    }

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
    // Get time since last paint, and reset timer
    int msec = m_qtTime.elapsed();
    m_qtTime.restart();

    // Update position of each channel
    VisualChannel *c;
    for (c = m_qlList.first(); c; c = m_qlList.next())
        c->move(msec);

    // Display stuff
    m_pVisualController->display();
}

void WVisualWaveform::resizeGL(int width, int height)
{
    m_pVisualController->resize((GLsizei)width,(GLsizei)height);
}

void WVisualWaveform::timerEvent(QTimerEvent*)
{
    updateGL();
}
