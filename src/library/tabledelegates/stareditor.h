#pragma once

#include <QWidget>
#include <QSize>
#include <QStyleOptionViewItem>
#include <QModelIndex>

class QTableView;

#include "library/starrating.h"

class StarEditor : public QWidget {
    Q_OBJECT

    // The isKeyboardEditMode property is used by the skin stylesheets
    // to distinguish StarEditor widgets with keyboard edit focus
    // from those without.
    Q_PROPERTY(bool isKeyboardEditMode MEMBER m_isKeyboardEditMode)

  public:
    StarEditor(QWidget* parent,
            QTableView* pTableView,
            const QModelIndex& index,
            const QStyleOptionViewItem& option,
            bool isKeyboardEditMode);

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

    QModelIndex getModelIndex() {
        return m_index;
    }

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
    StarRating m_starRating;
    int m_starCount;
    bool m_isKeyboardEditMode;
};
