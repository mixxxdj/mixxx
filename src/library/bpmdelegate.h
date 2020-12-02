#ifndef BPMDELEGATE_H
#define BPMDELEGATE_H

#include <QByteArrayData>
#include <QCheckBox>
#include <QModelIndex>
#include <QString>
#include <QStyleOptionViewItem>

#include "library/tableitemdelegate.h"

class QCheckBox;
class QItemEditorFactory;
class QModelIndex;
class QObject;
class QPainter;
class QTableView;

class BPMDelegate : public TableItemDelegate {
    Q_OBJECT
  public:
    explicit BPMDelegate(QTableView* pTableView);
    virtual ~BPMDelegate();

    void paintItem(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const;

  private:
    QTableView* m_pTableView;
    QCheckBox* m_pCheckBox;
    QItemEditorFactory* m_pFactory;
};

#endif // BPMDELEGATE_H
