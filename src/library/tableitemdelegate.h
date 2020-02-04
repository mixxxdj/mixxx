#pragma once

#include <QStyledItemDelegate>
#include <QTableView>


class TableItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
  public:
    explicit TableItemDelegate(QTableView* pTableView);
    ~TableItemDelegate() override = default;

    void paint(
            QPainter *painter,
            const QStyleOptionViewItem &option,
            const QModelIndex &index) const override;

    virtual void paintItem(
            QPainter *painter,
            const QStyleOptionViewItem &option,
            const QModelIndex &index) const = 0;

  protected:
    int columnWidth(const QModelIndex &index) const;

  private:
    QTableView* m_pTableView;
};
