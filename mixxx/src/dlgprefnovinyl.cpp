/***************************************************************************
                          DlgPrefNoVinyl.cpp  -  description
                             -------------------
    begin                : Thu Feb 24 2011
    copyright            : (C) 2011 by Owen Williams
    email                : owen-bugs@ywwg.com
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#include <QtCore>
#include <QtDebug>
#include <QtGui>
#include "dlgprefnovinyl.h"

DlgPrefNoVinyl::DlgPrefNoVinyl(QWidget * parent, SoundManager * soundman,
                               ConfigObject<ConfigValue> * _config)
        : QWidget(parent) {
    Q_UNUSED(soundman);
    Q_UNUSED(_config);
    setupUi(this);
}

DlgPrefNoVinyl::~DlgPrefNoVinyl()
{
}
