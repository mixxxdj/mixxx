/***************************************************************************
                          LibraryScannerDlg.cpp  -  shows library scanning
                                                       progress
                             -------------------
    begin                : 11/27/2007
    copyright            : (C) 2007 Albert Santoni and Adam Davison
    email                : gamegod \a\t users.sf.net
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtCore>
#include <QtDebug>
#include <QtGui>
#include "libraryscannerdlg.h"

bool LibraryScannerDlg::m_timersetup = false;
QTime LibraryScannerDlg::m_timer;
int LibraryScannerDlg::m_timeruses = 0;
QWidget* LibraryScannerDlg::m_progress = 0;
QLabel* LibraryScannerDlg::m_current = 0;
QPushButton* LibraryScannerDlg::m_cancel = 0;

LibraryScannerDlg::LibraryScannerDlg()
{
    m_timersetup = false;
    m_timeruses = 0;
    m_bCancelled = false;
    
    if (!m_timersetup) {
        m_timersetup = true;
	    setupTiming();
    }    
}

LibraryScannerDlg::~LibraryScannerDlg()
{

}

void LibraryScannerDlg::setupTiming() {
	m_timeruses = 0;
	m_progress = new QWidget();
	QVBoxLayout* layout = new QVBoxLayout();
	layout->addWidget(new QLabel(tr("It's taking Mixxx a minute to scan your music library, please wait...")));
	m_cancel = new QPushButton(tr("Cancel"), m_progress);
	layout->addWidget(m_cancel);
	m_current = new QLabel();
	layout->addWidget(m_current);
	m_progress->setLayout(layout);
	
	connect(m_cancel, SIGNAL(clicked()), this, SLOT(slotCancel())); 
}

void LibraryScannerDlg::slotStartTiming() {
	
	//qDebug() << "slotStartTiming!";
	m_qMutex.lock();
	
	if (m_timeruses == 0) {
        m_timer.start();
	}
	m_timeruses++;
	m_qMutex.unlock();
	
	//qDebug() << "Scanner timing started!";
	
}

void LibraryScannerDlg::slotStopTiming() {
	m_qMutex.lock();
	//qDebug() << "slotStopTiming!";
	
	m_timeruses--;
	if (m_timeruses == 0) {
		m_progress->setVisible(false);
	}
	m_qMutex.unlock();
}

void LibraryScannerDlg::slotCheckTiming(QString path) {

    m_qMutex.lock();
	//qDebug() << "******** slotCheckTiming!";
	
	if (!m_bCancelled && m_timer.elapsed() > 1000) {
		m_progress->setVisible(true);
	}

	if (m_progress->isVisible()) {
		m_current->setText("Scanning: " + path);
		m_current->repaint();
	}
	
	m_qMutex.unlock();

}

void LibraryScannerDlg::slotCancel()
{
    m_qMutex.lock();
    qDebug() << "Cancelling library scan...";
    m_bCancelled = true;
    m_qMutex.unlock();
     
    emit(scanCancelled());
    
    m_progress->setVisible(false);
}

