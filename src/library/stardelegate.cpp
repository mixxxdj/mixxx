/***************************************************************************
                           stardelegate.cpp
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


#include <QtDebug>

#include "library/tableitemdelegate.h"
#include "library/stardelegate.h"
#include "library/stareditor.h"
#include "library/starrating.h"

StarDelegate::StarDelegate(QTableView* pTableView)
        : TableItemDelegate(pTableView),
          m_pTableView(pTableView),
          m_isOneCellInEditMode(false) {
    connect(pTableView, SIGNAL(entered(QModelIndex)),
            this, SLOT(cellEntered(QModelIndex)));
}

void StarDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                         const QModelIndex& index) const {
    // let the editor do the painting if this cell is currently being edited
    if (index == m_currentEditedCellIndex) {
        return;
    }
    TableItemDelegate::paint(painter, option, index);
}

void StarDelegate::paintItem(QPainter* painter, const QStyleOptionViewItem& option,
                         const QModelIndex& index) const {
    // let the editor do the painting if this cell is currently being edited
    if (index == m_currentEditedCellIndex) {
        return;
    }

    StarRating starRating = index.data().value<StarRating>();
    starRating.paint(painter, option.rect);
}

QSize StarDelegate::sizeHint(const QStyleOptionViewItem& option,
                             const QModelIndex& index) const {
    Q_UNUSED(option);
    StarRating starRating = index.data().value<StarRating>();
    return starRating.sizeHint();
}

QWidget* StarDelegate::createEditor(QWidget* parent,
                                    const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const {
    // Populate the correct colors based on the styling
    QStyleOptionViewItem newOption = option;
    initStyleOption(&newOption, index);

    StarEditor* editor = new StarEditor(parent, m_pTableView, index, newOption);
    connect(editor, SIGNAL(editingFinished()),
            this, SLOT(commitAndCloseEditor()));
    return editor;
}

void StarDelegate::setEditorData(QWidget* editor,
                                 const QModelIndex& index) const {
    StarRating starRating = index.data().value<StarRating>();
    StarEditor* starEditor = qobject_cast<StarEditor*>(editor);
    starEditor->setStarRating(starRating);
}

void StarDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                const QModelIndex& index) const {
    StarEditor* starEditor = qobject_cast<StarEditor*>(editor);
    model->setData(index, qVariantFromValue(starEditor->starRating()));
}

void StarDelegate::commitAndCloseEditor() {
    StarEditor* editor = qobject_cast<StarEditor*>(sender());
    emit(commitData(editor));
    emit(closeEditor(editor));
}

void StarDelegate::cellEntered(const QModelIndex& index) {
    // This slot is called if the mouse pointer enters ANY cell on the
    // QTableView but the code should only be executed on a column with a
    // StarRating.
    if (index.data().canConvert(qMetaTypeId<StarRating>())) {
        if (m_isOneCellInEditMode) {
            m_pTableView->closePersistentEditor(m_currentEditedCellIndex);
        }
        m_pTableView->openPersistentEditor(index);
        m_isOneCellInEditMode = true;
        m_currentEditedCellIndex = index;
    } else if (m_isOneCellInEditMode) {
        m_isOneCellInEditMode = false;
        m_pTableView->closePersistentEditor(m_currentEditedCellIndex);
        m_currentEditedCellIndex = QModelIndex();
    }
}
