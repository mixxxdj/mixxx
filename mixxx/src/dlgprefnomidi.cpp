/***************************************************************************
                          dlgprefnomidi.cpp  -  
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "dlgprefnomidi.h"
#include <QtCore>
#include <QtGui>

DlgPrefNoMidi::DlgPrefNoMidi(QWidget * parent, ConfigObject<ConfigValue> * _config) :  QWidget(parent), Ui::DlgPrefNoMidiDlg()
{
    setupUi(this);
}

DlgPrefNoMidi::~DlgPrefNoMidi()
{
}