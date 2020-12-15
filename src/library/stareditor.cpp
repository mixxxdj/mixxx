#include "library/stareditor.h"

#include <QStylePainter>

#include "library/starrating.h"
#include "moc_stareditor.cpp"
#include "util/painterscope.h"

// We enable mouse tracking on the widget so we can follow the cursor even
// when the user doesn't hold down any mouse button. We also turn on
// QWidget's auto-fill background feature to obtain an opaque background.
// (Without the call, the view's background would shine through the editor.)

/// StarEditor inherits QWidget and is used by StarDelegate to let the user
/// edit a star rating in the library using the mouse.
///
/// The class has been adapted from the official "Star Delegate Example",
/// see http://doc.trolltech.com/4.5/itemviews-stardelegate.html
StarEditor::StarEditor(QWidget *parent, QTableView* pTableView,
                       const QModelIndex& index,
                       const QStyleOptionViewItem& option)
        : QWidget(parent),
          m_pTableView(pTableView),
          m_index(index),
          m_styleOption(option) {
    setMouseTracking(true);
}

QSize StarEditor::sizeHint() const {
    return m_starRating.sizeHint();
}

// static
void StarEditor::renderHelper(QPainter* painter,
                              QTableView* pTableView,
                              const QStyleOptionViewItem& option,
                              StarRating* pStarRating) {
    PainterScope painterScope(painter);

    painter->setClipRect(option.rect);

    if (pTableView != nullptr) {
        QStyle* style = pTableView->style();
        if (style != nullptr) {
            style->drawControl(QStyle::CE_ItemViewItem, &option, painter,
                               pTableView);
        }
    }

    // Set the palette appropriately based on whether the row is selected or
    // not. We also have to check if it is inactive or not and use the
    // appropriate ColorGroup.
    QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
            ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active)) {
        cg = QPalette::Inactive;
    }

    if (option.state & QStyle::State_Selected) {
        painter->setBrush(option.palette.color(cg, QPalette::HighlightedText));
    } else {
        painter->setBrush(option.palette.color(cg, QPalette::Text));
    }

    pStarRating->paint(painter, option.rect);
}

void StarEditor::paintEvent(QPaintEvent*) {
    // If a StarEditor is open, by definition the mouse is hovering over us.
    m_styleOption.state |= QStyle::State_MouseOver;
    m_styleOption.rect = rect();

    if (m_pTableView) {
        QItemSelectionModel* selectionModel = m_pTableView->selectionModel();
        if (selectionModel && selectionModel->isSelected(m_index)) {
            m_styleOption.state |= QStyle::State_Selected;
        }
    }

    QPainter painter(this);
    renderHelper(&painter, m_pTableView, m_styleOption, &m_starRating);
}

void StarEditor::mouseMoveEvent(QMouseEvent *event) {
    int star = starAtPosition(event->x());

    if (star != m_starRating.starCount() && star != -1) {
        m_starRating.setStarCount(star);
        update();
    }
}

void StarEditor::leaveEvent(QEvent*) {
    m_starRating.setStarCount(0);
    update();
}

void StarEditor::mouseReleaseEvent(QMouseEvent* /* event */) {
    emit editingFinished();
}

int StarEditor::starAtPosition(int x) {
    // If the mouse is very close to the left edge, set 0 stars.
    if (x < m_starRating.sizeHint().width() * 0.05) {
        return 0;
    }
    int star = (x / (m_starRating.sizeHint().width() / m_starRating.maxStarCount())) + 1;

    if (star <= 0 || star > m_starRating.maxStarCount()) {
        return 0;
    }
    return star;
}
