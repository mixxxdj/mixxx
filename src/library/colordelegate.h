#pragma once

#include <QByteArrayData>
#include <QModelIndex>
#include <QPainter>
#include <QString>
#include <QStyleOptionViewItem>

#include "library/tableitemdelegate.h"

class QModelIndex;
class QObject;
class QPainter;
class QTableView;

class ColorDelegate : public TableItemDelegate {
    Q_OBJECT
  public:
    explicit ColorDelegate(QTableView* pTableView);

    void paintItem(
            QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;
};
