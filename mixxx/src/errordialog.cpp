/***************************************************************************
                          errordialog.cpp  -  description
                             -------------------
    begin                : Sun Feb 22 2009
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

#include "errordialog.h"
#include <QMessageBox>
#include <QtDebug>
#include <QtCore>

ErrorDialog::ErrorDialog() {
    m_errorCondition=false;
    connect(this, SIGNAL(showErrorDialog(int, QString, QString)), this, SLOT(errorDialog(int, QString, QString)));
}

ErrorDialog::~ErrorDialog()
{
}

void ErrorDialog::requestErrorDialog(int type, QString message) {
    switch (type) {
        case 1: requestErrorDialog(type,"Mixxx - Critical error",message); break;
        case 0:
        default:
            requestErrorDialog(type,"Mixxx - Warning",message);
            break;
    }
}

void ErrorDialog::requestErrorDialog(int type, QString title, QString message) {
    emit (showErrorDialog(type, title, message));
    disconnect(this, SIGNAL(showErrorDialog(int, QString, QString)), this, SLOT(errorDialog(int, QString, QString))); // Avoid multiple dialogs until this one is acknowledged
}

void ErrorDialog::errorDialog(int type, QString title, QString message) {
    QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.setWindowTitle(title);
    switch (type) {
        case 1: msgBox.setIcon(QMessageBox::Critical); break;
        case 0:
        default:
            msgBox.setIcon(QMessageBox::Warning);
            break;
    }
    msgBox.exec();  // Block so the user can read it before exiting

    if (type==1) {  // If critical/fatal, gracefully exit application if possible
        m_errorCondition=true;
        if (QCoreApplication::instance()) {
            QCoreApplication::instance()->exit(-1);
        }
        else {
            qDebug() << "QCoreApplication::instance() is NULL! Abruptly quitting...";
            exit(-1);
        }
    }

    connect(this, SIGNAL(showErrorDialog(int, QString, QString)), this, SLOT(errorDialog(int, QString, QString)));    // reconnect the signal
}

bool ErrorDialog::checkError() {
    return m_errorCondition;
}