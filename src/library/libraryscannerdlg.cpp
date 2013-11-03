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

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtDebug>

#include "library/libraryscannerdlg.h"

LibraryScannerDlg::LibraryScannerDlg(QWidget * parent, Qt::WindowFlags f) :
    QWidget(parent, f),
    m_bCancelled(false),
    m_bPaused(false) {

    setWindowIcon(QIcon(":/images/ic_mixxx_window.png"));

    QVBoxLayout* pLayout = new QVBoxLayout(this);

    setWindowTitle(tr("Library Scanner"));
    QLabel* pLabel = new QLabel(tr("It's taking Mixxx a minute to scan your music library, please wait..."),this);
    pLayout->addWidget(pLabel);

    QPushButton* pCancel = new QPushButton(tr("Cancel"), this);
    connect(pCancel, SIGNAL(clicked()),
            this, SLOT(slotCancel()));
    pLayout->addWidget(pCancel);

    // pause button
    QPushButton* pPause = new QPushButton(tr("Pause"), this);
    pPause->setObjectName("pauseBtn");
    connect(pPause, SIGNAL(clicked()),
            this, SLOT(slotPause()));
    pLayout->addWidget(pPause);


    QLabel* pCurrent = new QLabel(this);
    pCurrent->setMaximumWidth(600);
    pCurrent->setMinimumHeight( pCurrent->fontMetrics().boundingRect("Gg").height()*2 );
    pCurrent->setMaximumHeight( pCurrent->fontMetrics().boundingRect("Gg").height() *3 );
    pCurrent->setWordWrap(true);
    connect(this, SIGNAL(progress(QString)),    // TODO(xxx) show progress on timer
            pCurrent, SLOT(setText(QString)));  // TODO(xxx) trim filename like "/home/tro/Music...09. Dope D.O.D. - Slowmotion.mp3"
    pLayout->addWidget(pCurrent);
    setLayout(pLayout);

    m_timer.start();
}

LibraryScannerDlg::~LibraryScannerDlg() {
    emit(scanCancelled());
}

void LibraryScannerDlg::slotUpdate(QString path) {
    //qDebug() << "LibraryScannerDlg slotUpdate" << m_timer.elapsed() << path;
    if (!m_bCancelled && m_timer.elapsed() > 2000) {
       setVisible(true);
    }

    if (isVisible()) {
        QString status = tr("Scanning: ") + path;
        emit(progress(status));
    }
}

void LibraryScannerDlg::slotCancel() {
    qDebug() << "Cancelling library scan...";
    m_bCancelled = true;

    emit(scanCancelled());

    // Need to use close() or else if you close the Mixxx window and then hit
    // Cancel, Mixxx will not shutdown.
    close();
}

void LibraryScannerDlg::slotPause() {
    QPushButton *pauseBtn = findChild<QPushButton*>("pauseBtn");
    if (!pauseBtn) {
        qDebug() << "\t ERROR: pauseBtn == NULL";
        return;
    }
    if (m_bPaused) {
        // resuming
        // qDebug() << "\t resuming";
        m_bPaused = false;
        pauseBtn->setText(tr("Pause"));
        emit(scanResumed());
    } else {
        // making pause
        // qDebug() << "\t making pause";
        pauseBtn->setText(tr("Resume"));
        m_bPaused = true;
        emit(scanPaused());
    }
}

void LibraryScannerDlg::slotScanFinished() {
    m_bCancelled = true; //Raise this flag to prevent any
                         //latent slotUpdates() from showing the dialog again.

    // Need to use close() or else if you close the Mixxx window and then hit
    // Cancel, Mixxx will not shutdown.
    close();
}
