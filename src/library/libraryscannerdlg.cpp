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

LibraryScannerDlg::LibraryScannerDlg(QWidget* parent, Qt::WindowFlags f)
        : QWidget(parent, f),
          m_bCancelled(false) {
    setWindowIcon(QIcon(":/images/ic_mixxx_window.png"));

    QVBoxLayout* pLayout = new QVBoxLayout(this);

    setWindowTitle(tr("Library Scanner"));
    QLabel* pLabel = new QLabel(tr("It's taking Mixxx a minute to scan your music library, please wait..."),this);
    pLayout->addWidget(pLabel);

    QPushButton* pCancel = new QPushButton(tr("Cancel"), this);
    connect(pCancel, SIGNAL(clicked()),
            this, SLOT(slotCancel()));
    pLayout->addWidget(pCancel);

    QLabel* pCurrent = new QLabel(this);
    pCurrent->setAlignment(Qt::AlignTop);
    pCurrent->setMaximumWidth(600);
    pCurrent->setFixedHeight(this->fontMetrics().height());
    pCurrent->setWordWrap(true);
    connect(this, SIGNAL(progress(QString)),
            pCurrent, SLOT(setText(QString)));
    pLayout->addWidget(pCurrent);
    setLayout(pLayout);
}

LibraryScannerDlg::~LibraryScannerDlg() {
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

void LibraryScannerDlg::slotUpdateCover(QString path) {
    //qDebug() << "LibraryScannerDlg slotUpdate" << m_timer.elapsed() << path;
    if (!m_bCancelled && m_timer.elapsed() > 2000) {
       setVisible(true);
    }

    if (isVisible()) {
        QString status = QString("%1: %2")
                .arg(tr("Scanning cover art (safe to cancel)"))
                .arg(path);
        emit(progress(status));
    }
}

void LibraryScannerDlg::slotCancel() {
    qDebug() << "Cancelling library scan...";
    m_bCancelled = true;
    emit(scanCancelled());
    hide();
}

void LibraryScannerDlg::slotScanStarted() {
    m_bCancelled = false;
    m_timer.start();
}

void LibraryScannerDlg::slotScanFinished() {
    // Raise this flag to prevent any latent slotUpdates() from showing the
    // dialog again.
    m_bCancelled = true;

    hide();
}
