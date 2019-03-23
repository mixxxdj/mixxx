#ifndef TABLEITEMDELEGATE_H
#define TABLEITEMDELEGATE_H

#include <QStyledItemDelegate>

class QTableView;
class QStyleOptionViewItem;

class TableItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
  public:
    explicit TableItemDelegate(QTableView* pTableView);
    virtual ~TableItemDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

    virtual void paintItem(QPainter *painter, const QStyleOptionViewItem &option,
            const QModelIndex &index) const = 0;

  private:
    QTableView* m_pTableView;
};

#endif // TABLEITEMDELEGATE_H
