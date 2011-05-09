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
#include <QDebug>

class ImgSource {
public:
    virtual ~ImgSource() {};
    virtual QImage* getImage(QString img) = 0;
    virtual inline QColor getCorrectColor(QColor c) { return c; }
};

class ImgProcessor : public ImgSource {

public:
    virtual ~ImgProcessor() {};
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
    virtual ~ImgColorProcessor() {};

    inline ImgColorProcessor(ImgSource* parent) : ImgProcessor(parent) {}
    inline virtual QImage* getImage(QString img) {
        QImage* i = m_parent->getImage(img);

        if (i == NULL || i->isNull()) {
            return i;
        }

        QColor col;

        int bytesPerPixel = 4;

        switch(i->format()) {
        case QImage::Format_Invalid:
          bytesPerPixel = 0;
          break;
        case QImage::Format_Mono:
        case QImage::Format_MonoLSB:
        case QImage::Format_Indexed8:
          bytesPerPixel = 1;
          break;

        case QImage::Format_RGB16:
        case QImage::Format_RGB555:
        case QImage::Format_RGB444:
        case QImage::Format_ARGB4444_Premultiplied:
          bytesPerPixel = 2;
          break;

        case QImage::Format_ARGB8565_Premultiplied:
        case QImage::Format_RGB666:
        case QImage::Format_ARGB6666_Premultiplied:
        case QImage::Format_ARGB8555_Premultiplied:
        case QImage::Format_RGB888:
          bytesPerPixel = 3;
          break;

        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied:
        case QImage::Format_RGB32:
        default:
          bytesPerPixel = 4;
          break;
        }

        //qDebug() << "ImgColorProcessor working on "
        //         << img << " bpp: "
        //         << bytesPerPixel << " format: " << i->format();

        if(bytesPerPixel < 4) {
          // Handling Indexed color or mono colors requires different logic
          qDebug() << "ImgColorProcessor aborting on unsupported color format:"
                   << i->format();
          return i;
        }

        for(int y = 0; y < i->height(); y++) {
          QRgb *line = (QRgb*)i->scanLine(y);

          if(line == NULL) {
            // Image is invalid.
            continue;
          }

          for(int x = 0; x < i->width(); x++) {
            col.setRgb(*line);
            col = doColorCorrection(col);
            *line = col.rgb();
            line++;
          }

        }

        return i;
    }
};

#endif

