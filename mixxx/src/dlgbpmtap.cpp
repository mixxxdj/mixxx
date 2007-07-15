/***************************************************************************
                          dlgpreferences.cpp  -  description
                             -------------------
    begin                : Sun Jun 30 2002
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

#ifdef __EXPERIMENTAL_BPM__
#include <qlabel.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qprogressbar.h>
#include <qspinbox.h>
#include <qdatetime.h>
#include "dlgbpmtap.h"
#include "mixxx.h"
#include "trackinfoobject.h"


DlgBPMTap::DlgBPMTap(QWidget *mixxx, TrackInfoObject *tio) : DlgBPMTapDlg(mixxx, "")
{
    //m_pMixxx = mixxx;
    m_CurrentTrack = tio;

    //Give focus to the tap button so that the tempo can be tapped with
    //the space bar
    btnTap->setFocus();
    
    //Create time object
    m_Time = new QTime(0,0);
    m_TapCount = 0;

    progressBPMDetect->setEnabled(false);
    radioBtnFast->setEnabled(false);
    radioBtnComplete->setEnabled(false);
    spinBoxBPMRangeStart->setEnabled(false);
    spinBoxBPMRangeEnd->setEnabled(false);
    btnGo->setEnabled(false);

    // Install event handler to generate closeDlg signal
    installEventFilter(this);

    // Connections
    connect(this,        SIGNAL(aboutToShow()),          this,      SLOT(slotLoadDialog()));
    connect(this,        SIGNAL(closeDlg()),             this,      SLOT(slotApply()));
    connect(btnTap,      SIGNAL(clicked()),              this,      SLOT(slotTapBPM()));
    connect(btnGo,       SIGNAL(clicked()),              this,      SLOT(slotDetectBPM()));
    connect(btnOK,       SIGNAL(clicked()),              this,      SLOT(slotOK()));

}

DlgBPMTap::~DlgBPMTap()
{
}

bool DlgBPMTap::eventFilter(QObject *o, QEvent *e)
{
    // Send a close signal if dialog is closing
    if (e->type() == QEvent::Hide)
        emit(closeDlg());

    if(e->type() == QEvent::Show)
        emit(aboutToShow());

    // Standard event processing
    return QWidget::eventFilter(o,e);
}

void DlgBPMTap::slotTapBPM()
{
	if(m_Time->elapsed() > 2000)
	{
		m_TapCount = 0;
	}

	if(m_TapCount <=0)
    {
        m_Time->restart();
    }
        
    if(m_TapCount > 0)
    {
        float elapsedTime = m_Time->elapsed() / (float)60000;

        float bpm = (float)m_TapCount / (float)elapsedTime;
        m_CurrentTrack->setBpm(bpm);
        txtBPM->setText(QString("%1").arg(bpm, 3,'f',1));
    }

    m_TapCount += 1;
}

void DlgBPMTap::slotDetectBPM()
{
}

void DlgBPMTap::slotLoadDialog()
{
    qDebug(m_CurrentTrack->getBpmStr());
    txtBPM->setText(m_CurrentTrack->getBpmStr());
}

void DlgBPMTap::slotOK()
{
    m_CurrentTrack->setBpm(txtBPM->text().toFloat());
    setHidden(true);
}

void DlgBPMTap::slotUpdate()
{
//    m_pMixxx->releaseKeyboard();
}

void DlgBPMTap::slotApply()
{
//    m_pMixxx->grabKeyboard();
}

#endif
