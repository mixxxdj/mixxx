/***************************************************************************
                           stardelegate.h
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


#ifndef STARDELEGATE_H
#define STARDELEGATE_H

#include <QStyledItemDelegate>
#include <QTableView>

class StarDelegate : public QStyledItemDelegate {
    Q_OBJECT
  public:
    StarDelegate(QObject* pParent = 0);

    // reimplemented from QItemDelegate and is called whenever the view needs to
    // repaint an item
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const;

    // returns an item's preferred size
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const;

    // called when the user starts editing an item
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const;

    // called when an editor is created to initialize it with data from the
    // model
    void setEditorData(QWidget* editor, const QModelIndex& index) const;

    // called when editing is finished, to commit data from the editor to the
    // model
    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const;

  private slots:
    void commitAndCloseEditor();
    void cellEntered(const QModelIndex& index);

  private:
    QTableView* m_pTableView;
    QPersistentModelIndex m_currentEditedCellIndex;
    bool m_isOneCellInEditMode;
};

#endif
