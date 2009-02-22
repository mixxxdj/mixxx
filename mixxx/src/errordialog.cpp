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

ErrorDialog::ErrorDialog() {
    connect(this, SIGNAL(showErrorDialog(int, QString)), this, SLOT(errorDialog(int, QString)));
    m_continue=false;
}

ErrorDialog::~ErrorDialog()
{
}

void ErrorDialog::requestErrorDialog(int type, QString message) {
    emit (showErrorDialog(type, message));
    while(!m_continue) ;
}

void ErrorDialog::errorDialog(int type, QString message) {
    switch (type) {
        case 1: QMessageBox::critical(0, "Mixxx", message); break;
        case 0:
        default: QMessageBox::warning(0, "Mixxx", message); break;
    }
    m_continue=true;
}
