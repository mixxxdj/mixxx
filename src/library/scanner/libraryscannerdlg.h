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

#ifndef LIBRARYSCANNERDLG_H
#define LIBRARYSCANNERDLG_H

#include <stddef.h>

#include <QString>
#include <QThread>
#include <QWidget>
#include <QtCore>

#include "util/performancetimer.h"

class LibraryScannerDlg : public QWidget {
    Q_OBJECT
  public:
    LibraryScannerDlg(QWidget* parent = NULL, Qt::WindowFlags f = Qt::Dialog);
    virtual ~LibraryScannerDlg();

  public slots:
    void slotUpdate(const QString& path);
    void slotUpdateCover(const QString& path);
    void slotCancel();
    void slotScanFinished();
    void slotScanStarted();

  signals:
    void scanCancelled();
    void progress(const QString&);

  private:
    PerformanceTimer m_timer;
    bool m_bCancelled;
};

#endif
