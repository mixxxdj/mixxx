/***************************************************************************
                          dlgprefkey.cpp  -  description
                             -------------------
    begin                : Thu Jun 7 2012
    copyright            : (C) 2012 by Keith Salisbury
    email                : keithsalisbury@gmail.com
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <qlineedit.h>
#include <qfiledialog.h>
#include <qwidget.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <QtCore>
#include <QMessageBox>

#include "dlgprefkey.h"
#include "xmlparse.h"

#define CONFIG_KEY "[KEY]"

DlgPrefKeyNotationFormat::DlgPrefKeyNotationFormat(QWidget * parent, 
  ConfigObject<ConfigValue> * _config) : QWidget(parent), 
  Ui::DlgPrefKeyNotationFormatDlg()
{
    config = _config;

    setupUi(this);
}

DlgPrefKeyNotationFormat::~DlgPrefKeyNotationFormat()
{
}

void DlgPrefKeyNotationFormat::slotApply()
{
}

void DlgPrefKeyNotationFormat::slotUpdate()
{
}
