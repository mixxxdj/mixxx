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
        int stars = m_starRating.starCount();
        VERIFY_OR_DEBUG_ASSERT(stars >= m_starRating.kMinStarCount &&
                stars <= m_starRating.maxStarCount()) {
            return;
        }
        m_starCount = stars;
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
    void leaveEvent(QEvent*) override;

  private:
    int starAtPosition(int x);
    void resetRating() {
        m_starRating.setStarCount(m_starCount);
        update();
    }

    QTableView* m_pTableView;
    QModelIndex m_index;
    QStyleOptionViewItem m_styleOption;
    StarRating m_starRating;
    int m_starCount;
};
