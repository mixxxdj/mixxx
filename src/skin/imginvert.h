/***************************************************************************
                          imginvert.h  -  description
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

#ifndef IMGINVERT_H
#define IMGINVERT_H

#include "imgsource.h"

class ImgInvert : public ImgColorProcessor {

public:
	inline ImgInvert(ImgSource* parent) : ImgColorProcessor(parent) {}
	virtual QColor doColorCorrection(QColor c);
};

#endif

