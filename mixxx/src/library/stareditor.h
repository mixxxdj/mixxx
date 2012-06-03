/***************************************************************************
                           stareditor.h
                              -------------------
     copyright            : (C) 2010 Tobias Rafreider
	 copyright            : (C) 2009 Nokia Corporation

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 * StarEditor inherits QWidget and is used by StarDelegate to let the user *
 * edit a star rating in the library using the mouse.                      *
 *																		   *
 * The class has been adapted from the official "Star Delegate Example",   *
 * see http://doc.trolltech.com/4.5/itemviews-stardelegate.html            *
 ***************************************************************************/

#ifndef STAREDITOR_H
#define STAREDITOR_H

#include <QWidget>

#include "starrating.h"

class StarEditor : public QWidget {
    Q_OBJECT
  public:
    StarEditor(QWidget *parent, const QStyleOptionViewItem& option);

    QSize sizeHint() const;
    void setStarRating(const StarRating &starRating) {
        m_starRating = starRating;
    }
    StarRating starRating() { return m_starRating; }

  signals:
    void editingFinished();

  protected:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

  private:
    int starAtPosition(int x);

    StarRating m_starRating;
    bool m_isSelected;
};

#endif
