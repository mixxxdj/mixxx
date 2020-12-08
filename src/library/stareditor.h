#pragma once

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
