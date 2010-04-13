/***************************************************************************
                    messageboxhandler.cpp  - allows non-gui threads to
                                             make QMessageBoxes
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

#include "messageboxhandler.h"
#include <QMessageBox>
#include <QtCore>
#include <QtDebug>   // for qDebug

MessageBoxHandler::MessageBoxHandler() {
//     qRegisterMetaType<QMessageBox&>("QMessageBox&");
    connect(this, SIGNAL(showMessageBox(QMessageBox&,bool)), this, SLOT(messageBox(QMessageBox&,bool)));
}

MessageBoxHandler::~MessageBoxHandler()
{
}

QMessageBox& MessageBoxHandler::newOne() {
    QMessageBox& msgBox = *new QMessageBox();
    qDebug() << "New QMessageBox created";
//         printf("%08X", &msgBox);
    return msgBox;
}

void MessageBoxHandler::display(QMessageBox& msgBox, bool modal) {
    emit(showMessageBox(msgBox, modal));
}

void MessageBoxHandler::messageBox(QMessageBox& msgBox, bool modal) {
    if (modal) msgBox.exec();
    else msgBox.show();
}

int MessageBoxHandler::getResult(QMessageBox& msgBox) {
    return msgBox.result();
}

void MessageBoxHandler::cleanup(QMessageBox& msgBox) {
    qDebug() << "Deleting QMessageBox";
//         printf("%08X", &msgBox);
    QMessageBox* temp = &msgBox;
//     &msgBox = NULL;  // this doesn't work
    delete temp;
}