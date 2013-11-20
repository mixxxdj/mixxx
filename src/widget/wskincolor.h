/***************************************************************************
                          wskincolor.h  -  description
                             -------------------
    begin                : 14 April 2007
    copyright            : (C) 2007 by Adam Davison
    email                : adamdavison@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WSKINCOLOR_H
#define WSKINCOLOR_H

#include <QSharedPointer>
#include "skin/imgsource.h"

class WSkinColor {
  public:
    static QColor getCorrectColor(QColor c);
    static void setLoader(QSharedPointer<ImgSource> ld);
  private:
    static QSharedPointer<ImgSource> loader;
};

#endif

