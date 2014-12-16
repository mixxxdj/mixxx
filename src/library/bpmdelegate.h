#ifndef BPMDELEGATE_H
#define BPMDELEGATE_H

#include <QCheckBox>
#include <QModelIndex>
#include <QObject>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>
#include <QTableView>

class BPMDelegate : public QStyledItemDelegate {
    Q_OBJECT
  public:
    explicit BPMDelegate(QObject* parent);
    virtual ~BPMDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

  private:
    QTableView* m_pTableView;
    QCheckBox* m_pCheckBox;
};

#endif // BUTTONCOLUMNDELEGATE_H
