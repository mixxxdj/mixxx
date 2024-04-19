#include "library/tabledelegates/stareditor.h"

#include <QItemSelectionModel>
#include <QMouseEvent>
#include <QPainter>
#include <QTableView>

#include "library/starrating.h"
#include "library/tabledelegates/tableitemdelegate.h"
#include "moc_stareditor.cpp"

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
        const QStyleOptionViewItem& option,
        const QColor& focusBorderColor)
        : QWidget(parent),
          m_pTableView(pTableView),
          m_index(index),
          m_styleOption(option),
          m_focusBorderColor(focusBorderColor),
          m_starCount(StarRating::kMinStarCount) {
    DEBUG_ASSERT(m_pTableView);
    setMouseTracking(true);
    installEventFilter(this);
}

QSize StarEditor::sizeHint() const {
    return m_starRating.sizeHint();
}

void StarEditor::paintEvent(QPaintEvent*) {
    m_styleOption.rect = rect();

    if (underMouse()) {
        m_styleOption.state |= QStyle::State_MouseOver;
    } else {
        m_styleOption.state &= ~QStyle::State_MouseOver;
    }

    QItemSelectionModel* selectionModel = m_pTableView->selectionModel();
    if (selectionModel) {
        // If the editor cell is selected set the respective flag so we can use the
        // palette's 'HighlightedText' font color for the brush StarRating will use
        // to fill the star/diamond polygons with.
        // Else, unset it and we use the regular color.
        if (selectionModel->isSelected(m_index)) {
            m_styleOption.state |= QStyle::State_Selected;
        } else {
            m_styleOption.state &= ~QStyle::State_Selected;
        }
        // Accordingly, un/set the focus flag.
        if (selectionModel->currentIndex() == m_index) {
            m_styleOption.state |= QStyle::State_HasFocus;
        } else {
            m_styleOption.state &= ~QStyle::State_HasFocus;
        }
    }

    QPainter painter(this);

    painter.setClipRect(m_styleOption.rect);

    // Draw standard item with the table view's style
    QStyle* style = m_pTableView->style();
    if (style) {
        style->drawControl(QStyle::CE_ItemViewItem, &m_styleOption, &painter, m_pTableView);
    }

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

    // Draw a border if the color cell is selected
    if (m_styleOption.state & QStyle::State_HasFocus) {
        TableItemDelegate::drawBorder(&painter, m_focusBorderColor, m_styleOption.rect);
    }
}

bool StarEditor::eventFilter(QObject* obj, QEvent* event) {
    switch (event->type()) {
    case QEvent::KeyPress: {
        VERIFY_OR_DEBUG_ASSERT(m_isKeyboardEditMode) {
            // Persistent editors (i.e. those opened using openPersistentEditor)
            // should not receive keyboard events via their eventFilter, so we
            // shouldn't normally arrive here - but if we do, just forward the events.
            return false;
        }

        // Change rating when certain keys are pressed
        // while we are in edit mode.
        QKeyEvent* ke = static_cast<QKeyEvent*>(event);
        int newRating = m_starRating.starCount();
        switch (ke->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown: {
            // Allow handling of certain keyboard navigation events.
            // This logic attempts to match the behavior for text editor cells.
            return false;
        }
        case Qt::Key_Home:
        case Qt::Key_End: {
            // Only forward Home and End if the Ctrl key is pressed.
            // This matches the behavior of normal text editor cells.
            if (ke->modifiers() & Qt::ControlModifier) {
                return false;
            }
            return true;
        }
        case Qt::Key_0: {
            newRating = 0;
            break;
        }
        case Qt::Key_1: {
            newRating = 1;
            break;
        }
        case Qt::Key_2: {
            newRating = 2;
            break;
        }
        case Qt::Key_3: {
            newRating = 3;
            break;
        }
        case Qt::Key_4: {
            newRating = 4;
            break;
        }
        case Qt::Key_5: {
            newRating = 5;
            break;
        }
        case Qt::Key_6: {
            newRating = 6;
            break;
        }
        case Qt::Key_7: {
            newRating = 7;
            break;
        }
        case Qt::Key_8: {
            newRating = 8;
            break;
        }
        case Qt::Key_9: {
            newRating = 9;
            break;
        }
        case Qt::Key_Right:
        case Qt::Key_Plus: {
            newRating++;
            break;
        }
        case Qt::Key_Left:
        case Qt::Key_Minus: {
            newRating--;
            break;
        }
        }
        newRating = math_clamp(newRating, StarRating::kMinStarCount, m_starRating.maxStarCount());
        if (newRating != m_starRating.starCount()) {
            // Apply star rating if it changed
            m_starRating.setStarCount(newRating);
            m_starCount = newRating;
            update();
        }
        // Prevent other keys from being handled as global keyboard shortcuts.
        // This matches the behavior of the other edit controls.
        return true;
    }
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
