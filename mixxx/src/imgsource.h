/***************************************************************************
                          imgsource.h  -  description
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

#ifndef IMGSOURCE_H
#define IMGSOURCE_H

#include <qimage.h>
#include <qcolor.h>

class ImgSource {

public:
	virtual QImage* getImage(QString img) = 0;
	virtual inline QColor getCorrectColor(QColor c) { return c; }
};

class ImgProcessor : public ImgSource {

public:
	inline ImgProcessor(ImgSource* parent) : m_parent(parent) {}
	virtual QColor doColorCorrection(QColor c) = 0;
	inline QColor getCorrectColor(QColor c) {
		return doColorCorrection(m_parent->getCorrectColor(c));
	}

protected:
	ImgSource* m_parent;
};

class ImgColorProcessor : public ImgProcessor {

public:
	inline ImgColorProcessor(ImgSource* parent) : ImgProcessor(parent) {}
	inline virtual QImage* getImage(QString img) {
		QImage* i = m_parent->getImage(img);

		QColor col;

		for (int x = 0; x < i->width(); x++) {
			for (int y = 0; y < i->height(); y++) {
				QRgb pix = i->pixel(x, y);
				col.setRgb(pix);
				col = doColorCorrection(col);
				i->setPixel(x, y, col.rgb());
			}
		}

		return i;
	}
};

#endif

