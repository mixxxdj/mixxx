/***************************************************************************
                    messageboxhandler.h  -  description
                             -------------------
    begin                : Tue Mar 23 2010
    copyright            : (C) 2010 by Sean M. Pappalardo
    email                : spappalardo@mixxx.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MESSAGEBOXHANDLER_H
#define MESSAGEBOXHANDLER_H

#include <QMessageBox>

/**
  * Class used to allow all threads (notably non-gui ones) to display custom 
  * QMessageBoxes
  *
  *@author Sean M. Pappalardo
  */

class MessageBoxHandler : public QObject {
    Q_OBJECT
public:
    MessageBoxHandler();
    ~MessageBoxHandler();
    QMessageBox& newOne();
    /** Call this to emit a signal to display the referenced message box */
    void display(QMessageBox& msgBox, bool modal);
    int getResult(QMessageBox& msgBox);
    void cleanup(QMessageBox& msgBox);

signals:
    void showMessageBox(QMessageBox& msgBox, bool modal);

private slots:
    /** Actually displays the box */
    void messageBox(QMessageBox& msgBox, bool modal);
};

extern MessageBoxHandler* g_pMessageBoxHelper;

#endif
