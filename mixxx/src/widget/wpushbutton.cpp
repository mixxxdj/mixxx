/***************************************************************************
                          wpushbutton.cpp  -  description
                             -------------------
    begin                : Fri Jun 21 2002
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

#include "wpushbutton.h"
#include "wpixmapstore.h"
#include "controlobject.h"
#include "controlpushbutton.h"
//Added by qt3to4:
#include <QPixmap>
#include <QtDebug>
#include <QMouseEvent>
#include <QPaintEvent>

WPushButton::WPushButton(QWidget * parent) : WWidget(parent)
{
    m_pPixmaps = 0;
    m_pPixmapBack = 0;
    setStates(0);
    //setBackgroundMode(Qt::NoBackground); //obsolete? removal doesn't seem to change anything on the GUI --kousu 2009/03 
}

WPushButton::~WPushButton()
{
    for (int i = 0; i < 2*m_iNoStates; i++) {
        WPixmapStore::deletePixmap(m_pPixmaps[i]);
    }

    WPixmapStore::deletePixmap(m_pPixmapBack);
}

void WPushButton::setup(QDomNode node)
{
    // Number of states
    int iNumStates = selectNodeInt(node, "NumberStates");
    setStates(iNumStates);

    // Set background pixmap if available
    if (!selectNode(node, "BackPath").isNull())
        setPixmapBackground(getPath(selectNodeQString(node, "BackPath")));

    // Load pixmaps for associated states
    QDomNode state = selectNode(node, "State");
    while (!state.isNull())
    {
        if (state.isElement() && state.nodeName() == "State")
        {
            setPixmap(selectNodeInt(state, "Number"), true, getPath(selectNodeQString(state, "Pressed")));
            setPixmap(selectNodeInt(state, "Number"), false, getPath(selectNodeQString(state, "Unpressed")));
        }
        state = state.nextSibling();
    }

    //--------
    //This next big block allows each ControlPushButton to know whether or not it's
    //a "toggle" button.

    // For each connection
    QDomNode con = selectNode(node, "Connection");
    while (!con.isNull())
    {
        // Get ConfigKey
        QString key = selectNodeQString(con, "ConfigKey");

        ConfigKey configKey;
        configKey.group = key.left(key.indexOf(","));
        configKey.item = key.mid(key.indexOf(",")+1);

        ControlPushButton * p = (ControlPushButton *)ControlObject::getControl(configKey);

        //Find out if we're a push button...
        if (node.nodeName()=="PushButton")
        {
            //qDebug(configKey.item);
            //qDebug() << "Number of States: " << iStates;

            //If we have 2 states, tell my controlpushbutton object that we're a toggle button.
          if (iNumStates == 2) {
            if (p == 0)
              qDebug() << "Warning: wpushbutton p is null\n";
            else
              p->setToggleButton(true);
          }
            // BJW: Removed this so that buttons that are hardcoded as toggle in the source
            // don't get overridden if a skin fails to set them to 2-state. Buttons still
            // default to non-toggle otherwise.
            // else
            //	p->setToggleButton(false);
        }

        con = con.nextSibling();
    }

    //End of toggle button stuff.
    //--------


    // Setup position
    WWidget::setup(node);
}

void WPushButton::setStates(int iStates)
{
    m_iNoStates = iStates;
    m_fValue = 0.;
    m_bPressed = false;

    // If pixmap array is already allocated, delete it
    if (m_pPixmaps)
        delete [] m_pPixmaps;

    if (iStates>0)
    {
        m_pPixmaps = new QPixmap*[2*m_iNoStates];
        for (int i=0; i<2*m_iNoStates; ++i)
            m_pPixmaps[i] = 0;
    }
}

void WPushButton::setPixmap(int iState, bool bPressed, const QString &filename)
{
    int pixIdx = (iState*2)+bPressed;
    m_pPixmaps[pixIdx] = WPixmapStore::getPixmap(filename);
    if (!m_pPixmaps[pixIdx])
        qDebug() << "WPushButton: Error loading pixmap:" << filename;

    // Set size of widget equal to pixmap size
    setFixedSize(m_pPixmaps[pixIdx]->size());
}

void WPushButton::setPixmapBackground(const QString &filename)
{
    // Load background pixmap
    m_pPixmapBack = WPixmapStore::getPixmap(filename);
    if (!m_pPixmapBack)
        qDebug() << "WPushButton: Error loading background pixmap:" << filename;
}

void WPushButton::setValue(double v)
{
    m_fValue = v;

    if (m_iNoStates==1)
    {
        if (m_fValue==1.)
            m_bPressed = true;
        else
            m_bPressed = false;
    }

    update();
}

void WPushButton::paintEvent(QPaintEvent *)
{
    if (m_iNoStates>0)
    {
        int idx = (((int)m_fValue%m_iNoStates)*2)+m_bPressed;
        if (m_pPixmaps[idx])
        {
            QPainter p(this);
            if(m_pPixmapBack) p.drawPixmap(0, 0, *m_pPixmapBack);
            p.drawPixmap(0, 0, *m_pPixmaps[idx]);
        }
    }
}

void WPushButton::mousePressEvent(QMouseEvent * e)
{
    m_bPressed = true;

    // Calculate new state if it is a one state button
    if (m_iNoStates==1)
    {
        if (m_fValue==0.)
            m_fValue = 1.;
        else
            m_fValue = 0.;
    }
    // Update state on left press if it is a n-state button
    else if (e->button()==Qt::LeftButton)
    {
        m_fValue = (int)(m_fValue+1.)%m_iNoStates;
    }
    
    if (e->button()==Qt::LeftButton)
        emit(valueChangedLeftDown((double)m_fValue));
    else if (e->button()==Qt::RightButton)
        emit(valueChangedRightDown((double)m_fValue));

    update();
}

void WPushButton::mouseReleaseEvent(QMouseEvent * e)
{
    m_bPressed = false;

    // Update state if it is a one state button.
    if (m_iNoStates==1) // && e->button()==Qt::LeftButton)
    {
        if (m_fValue==0.)
            m_fValue = 1.;
        else
            m_fValue = 0.;
    }

    if (e->button()==Qt::LeftButton)
        emit(valueChangedLeftUp((double)m_fValue));
    else if (e->button()==Qt::RightButton)
        emit(valueChangedRightUp((double)m_fValue));

    update();
}
