#pragma once

#include <QWidget>
#include <QSize>
#include <QStyleOptionViewItem>
#include <QModelIndex>

class QTableView;

#include "library/starrating.h"

class StarEditor : public QWidget {
    Q_OBJECT
  public:
    StarEditor(QWidget* parent, QTableView* pTableView,
               const QModelIndex& index,
               const QStyleOptionViewItem& option);

    QSize sizeHint() const override;
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
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    //if the mouse leaves the editing index set starCount to 0
    void leaveEvent(QEvent*) override;

  private:
    int starAtPosition(int x);

    QTableView* m_pTableView;
    QModelIndex m_index;
    QStyleOptionViewItem m_styleOption;
    StarRating m_starRating;
};
