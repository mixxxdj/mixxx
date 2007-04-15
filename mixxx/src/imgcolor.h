/***************************************************************************
                          imgcolor.h  -  description
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

#ifndef IMGCOLOR_H
#define IMGCOLOR_H

#include "imgsource.h"

class ImgAdd : public ImgColorProcessor {

public:
	ImgAdd(ImgSource* parent, int amt);
	virtual QColor doColorCorrection(QColor c);

private:
	int m_amt;
};

class ImgMax : public ImgColorProcessor {

public:
	ImgMax(ImgSource* parent, int amt);
	virtual QColor doColorCorrection(QColor c);

private:
	int m_amt;
};

class ImgScaleWhite : public ImgColorProcessor {

public:
	inline ImgScaleWhite(ImgSource* parent, float amt)
		: ImgColorProcessor(parent), m_amt(amt) {}
	virtual QColor doColorCorrection(QColor c);
private:
	float m_amt;
};

class ImgHueRot : public ImgColorProcessor {

public:
	inline ImgHueRot(ImgSource* parent, int amt)
		: ImgColorProcessor(parent), m_amt(amt) {}
	virtual QColor doColorCorrection(QColor c);

private:
	int m_amt;
};

class ImgHueInv : public ImgColorProcessor {

public:
	inline ImgHueInv(ImgSource* parent) : ImgColorProcessor(parent) {}
	virtual QColor doColorCorrection(QColor c);
};
#endif