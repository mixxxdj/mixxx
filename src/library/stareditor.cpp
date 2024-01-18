#include "library/stareditor.h"

#include <QItemSelectionModel>
#include <QMouseEvent>
#include <QTableView>

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
StarEditor::StarEditor(QWidget* parent,
        QTableView* pTableView,
        const QModelIndex& index,
        const QStyleOptionViewItem& option)
        : QWidget(parent),
          m_pTableView(pTableView),
          m_index(index),
          m_styleOption(option),
          m_starCount(StarRating::kMinStarCount) {
    DEBUG_ASSERT(m_pTableView);
    setMouseTracking(true);
    installEventFilter(this);
}

QSize StarEditor::sizeHint() const {
    return m_starRating.sizeHint();
}

void StarEditor::paintEvent(QPaintEvent*) {
    // If a StarEditor is open, by definition the mouse is hovering over us.
    m_styleOption.state |= QStyle::State_MouseOver;
    m_styleOption.rect = rect();

    // If the editor cell is selected set the respective flag so we can use the
    // palette's 'HighlightedText' font color for the brush StarRating will use
    // to fill the star/diamond polygons with.
    QItemSelectionModel* selectionModel = m_pTableView->selectionModel();
    if (selectionModel && selectionModel->isSelected(m_index)) {
        m_styleOption.state |= QStyle::State_Selected;
    }

    QPainter painter(this);
    PainterScope painterScope(&painter);

    painter.setClipRect(m_styleOption.rect);

    // Draw standard item with the table view's style
    QStyle* style = m_pTableView->style();
    if (style) {
        style->drawControl(QStyle::CE_ItemViewItem, &m_styleOption, &painter, m_pTableView);
    }

    // Starrating scales the painter so do this after painting the border.
    // Set the palette appropriately based on whether the row is selected or
    // not. We also have to check if it is inactive or not and use the
    // appropriate ColorGroup.
    QPalette::ColorGroup cg = m_styleOption.state & QStyle::State_Enabled
            ? QPalette::Normal
            : QPalette::Disabled;
    if (cg == QPalette::Normal && !(m_styleOption.state & QStyle::State_Active)) {
        cg = QPalette::Inactive;
    }

    if (m_styleOption.state & QStyle::State_Selected) {
        painter.setBrush(m_styleOption.palette.color(cg, QPalette::HighlightedText));
    } else {
        painter.setBrush(m_styleOption.palette.color(cg, QPalette::Text));
    }

    m_starRating.paint(&painter, m_styleOption.rect);
}

bool StarEditor::eventFilter(QObject* obj, QEvent* event) {
    switch (event->type()) {
    case QEvent::Leave:
    case QEvent::ContextMenu: {
        // Note: it seems with Qt5 we do not reliably get a Leave event when
        // invoking the track menu via right click, so reset the rating now.
        // The event is forwarded to parent QTableView.
        resetRating();
        break;
    }
    case QEvent::MouseButtonRelease: {
        emit editingFinished();
        break;
    }
    case QEvent::MouseMove: {
        // Change rating only if no button is pressed.
        // This allows dragging the row also by grabbing the star cell
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (me->buttons().testFlag(Qt::NoButton)) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            const int eventPosition = static_cast<int>(me->position().x());
#else
            const int eventPosition = me->x();
#endif
            int star = m_starRating.starAtPosition(eventPosition, m_styleOption.rect);

            if (star <= StarRating::kInvalidStarCount) {
                resetRating();
            } else if (star != m_starRating.starCount()) {
                // Apply star rating if it changed
                m_starRating.setStarCount(star);
                update();
            }
        }
        break;
    }
    default:
        break;
    }

    return QWidget::eventFilter(obj, event);
}
