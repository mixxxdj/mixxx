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

    // Calculate the width/height of a single star rectangle from the
    // used library font..
    // Half the font height is roughly equal to the default size (15px) with
    // Open Sans regular 11pt.
    static int starBreadthFromFont(const QStyleOption& option) {
        auto mf = option.fontMetrics;
        return static_cast<int>((mf.capHeight() + mf.ascent()) / 2);
    }

    void setStarRating(const StarRating& starRating) {
        m_starRating = starRating;
        m_starRating.setStarBreadth(starBreadthFromFont(m_styleOption));
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
    QColor m_pFocusBorderColor;
    StarRating m_starRating;
    int m_starCount;
};
