#pragma once

#include "library/tableitemdelegate.h"

class StarDelegate : public TableItemDelegate {
    Q_OBJECT
  public:
    StarDelegate(QTableView* pTrackTable);

    // reimplemented from QItemDelegate and is called whenever the view needs to
    // repaint an item
    void paintItem(QPainter* painter, const QStyleOptionViewItem& option,
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
