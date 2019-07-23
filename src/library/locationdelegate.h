#ifndef LOCATIONDELEGATE_H
#define LOCATIONDELEGATE_H

#include <QStyledItemDelegate>

class QTableView;
class QStyleOptionViewItem;

class LocationDelegate : public QStyledItemDelegate {
    Q_OBJECT
  public:
    LocationDelegate(QTableView* pTrackTable);

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const;

  private:
    QTableView* m_pTableView;
};

#endif /* LOCATIONDELEGATE_H */
