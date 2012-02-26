/***************************************************************************
                          dlgabout.cpp  -  description
                             -------------------
    begin                : Mon Nov 19 2007
    copyright            : (C) 2007 by Albert Santoni
    email                : gamegod at users.sf.net
 ***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "dlgabout.h"

#include <qlineedit.h>
#include <qwidget.h>
#include <qslider.h>
#include <qlabel.h>
#include <qstring.h>
#include <qpushbutton.h>

DlgAbout::DlgAbout(QWidget* parent) :  QDialog(parent), Ui::DlgAboutDlg() {
    setupUi(this);
}

DlgAbout::~DlgAbout() {
}

void DlgAbout::slotApply() {
}

void DlgAbout::slotUpdate() {
}

