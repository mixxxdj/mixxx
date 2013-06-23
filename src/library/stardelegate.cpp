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
#include <QtGui>

#include "stardelegate.h"
#include "stareditor.h"
#include "starrating.h"

StarDelegate::StarDelegate(QObject *pParent)
        : QStyledItemDelegate(pParent),
          m_isOneCellInEditMode(false) {
    m_pTableView = qobject_cast<QTableView *>(pParent);
    connect(pParent, SIGNAL(entered(QModelIndex)),
            this, SLOT(cellEntered(QModelIndex)));
}

void StarDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //let the editor to the painting if this is true
    if(index==m_currentEditedCellIndex){
        return;
    }

    // Populate the correct colors based on the styling
    QStyleOptionViewItem newOption = option;
    initStyleOption(&newOption, index);

    // Set the palette appropriately based on whether the row is selected or
    // not. We also have to check if it is inactive or not and use the
    // appropriate ColorGroup.
    if (newOption.state & QStyle::State_Selected) {
        QPalette::ColorGroup colorGroup =
                newOption.state & QStyle::State_Active ?
                QPalette::Active : QPalette::Inactive;
        painter->fillRect(newOption.rect,
            newOption.palette.color(colorGroup, QPalette::Highlight));
        painter->setBrush(newOption.palette.color(
            colorGroup, QPalette::HighlightedText));
    } else {
        painter->fillRect(newOption.rect, newOption.palette.base());
        painter->setBrush(newOption.palette.text());
    }

    StarRating starRating = qVariantValue<StarRating>(index.data());
    starRating.paint(painter, newOption.rect, newOption.palette, StarRating::ReadOnly,
                     newOption.state & QStyle::State_Selected);
}

QSize StarDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    StarRating starRating = qVariantValue<StarRating>(index.data());
    return starRating.sizeHint();
}

QWidget *StarDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,const QModelIndex &index) const
{
    // Populate the correct colors based on the styling
    QStyleOptionViewItem newOption = option;
    initStyleOption(&newOption, index);

    StarEditor *editor = new StarEditor(parent, newOption);
    connect(editor, SIGNAL(editingFinished()),
            this, SLOT(commitAndCloseEditor()));
    return editor;
}

void StarDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    StarRating starRating = qVariantValue<StarRating>(index.data());
    StarEditor *starEditor = qobject_cast<StarEditor *>(editor);
    starEditor->setStarRating(starRating);
}

void StarDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    StarEditor *starEditor = qobject_cast<StarEditor *>(editor);
    model->setData(index, qVariantFromValue(starEditor->starRating()));
}

/*
 * When the user is done editing, we emit commitData() and closeEditor() (both declared in QAbstractItemDelegate),
 * to tell the model that there is edited data and to inform the view that the editor is no longer needed.
 */
void StarDelegate::commitAndCloseEditor() {
    StarEditor *editor = qobject_cast<StarEditor *>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}

//cellEntered
void StarDelegate::cellEntered(const QModelIndex &index) {
    // This slot is called if the mouse pointer enters ANY cell on the
    // QTableView but the code should only be executed on a column with a
    // StarRating.
    if (qVariantCanConvert<StarRating>(index.data())) {
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

