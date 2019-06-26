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
 *                                                                         *
 * The class has been adapted from the official "Star Delegate Example",   *
 * see http://doc.trolltech.com/4.5/itemviews-stardelegate.html            *
 ***************************************************************************/

#ifndef STAREDITOR_H
#define STAREDITOR_H

#include <QWidget>
#include <QMouseEvent>
#include <QEvent>
#include <QStyle>
#include <QSize>
#include <QPaintEvent>
#include <QStyleOptionViewItem>
#include <QTableView>
#include <QModelIndex>

#include "library/starrating.h"

class StarEditor : public QWidget {
    Q_OBJECT
  public:
    StarEditor(QWidget* parent, QTableView* pTableView,
               const QModelIndex& index,
               const QStyleOptionViewItem& option);

    QSize sizeHint() const;
    void setStarRating(const StarRating& starRating) {
        m_starRating = starRating;
    }
    StarRating starRating() { return m_starRating; }

    static void renderHelper(QPainter* painter, QTableView* pTableView,
                             const QStyleOptionViewItem& option,
                             StarRating* pStarRating);

  signals:
    void editingFinished();

  protected:
    void paintEvent(QPaintEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    //if the mouse leaves the editing index set starCount to 0
    void leaveEvent(QEvent*);

  private:
    int starAtPosition(int x);

    QTableView* m_pTableView;
    QModelIndex m_index;
    QStyleOptionViewItem m_styleOption;
    StarRating m_starRating;
};

#endif
