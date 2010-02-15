/***************************************************************************
                          errordialog.h  -  description
                             -------------------
    begin                : Fri Feb 20 2009
    copyright            : (C) 2009 by Sean M. Pappalardo
    email                : pegasus@c64.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QObject>

/**
  * Class used to allow all threads to display message boxes on error conditions
  *
  *@author Sean M. Pappalardo
  */

class ErrorDialog : public QObject {
   Q_OBJECT
public:
    ErrorDialog();
    ~ErrorDialog();
    /** A qMessageHandler calls either of these to emit a signal to display the requested message box */
    void requestErrorDialog(int type, QString message);
    void requestErrorDialog(int type, QString title, QString message);
    /** Allows a means for main() to skip exec() if there was a critical or fatal error dialog displayed on app initialization */
    bool checkError();

signals:
    void showErrorDialog(int type, QString title, QString message);

private:
    bool m_errorCondition;

private slots:
    /** Actually displays the box */
    void errorDialog(int type, QString title, QString message);
};

#endif
