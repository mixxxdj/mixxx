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
    StarEditor(QWidget* parent,
            QTableView* pTableView,
            const QModelIndex& index,
            const QStyleOptionViewItem& option,
            const QColor& focusBorderColor);

    QSize sizeHint() const override;
    void setStarRating(const StarRating& starRating) {
        m_starRating = starRating;
        int stars = m_starRating.starCount();
        VERIFY_OR_DEBUG_ASSERT(m_starRating.verifyStarCount(stars)) {
            return;
        }
        m_starCount = stars;
    }
    StarRating starRating() { return m_starRating; }

  signals:
    void editingFinished();

  protected:
    void paintEvent(QPaintEvent* event) override;

    bool eventFilter(QObject* obj, QEvent* event) override;

  private:
    void resetRating() {
        m_starRating.setStarCount(m_starCount);
        update();
    }

    QTableView* m_pTableView;
    QModelIndex m_index;
    QStyleOptionViewItem m_styleOption;
    QColor m_focusBorderColor;
    StarRating m_starRating;
    int m_starCount;
};
