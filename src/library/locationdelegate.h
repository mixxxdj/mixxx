#pragma once

#include <QByteArrayData>
#include <QString>
#include <QStyleOptionViewItem>

#include "library/tableitemdelegate.h"

class QModelIndex;
class QObject;
class QPainter;
class QTableView;

class LocationDelegate : public TableItemDelegate {
    Q_OBJECT
  public:
    explicit LocationDelegate(QTableView* pTrackTable);
    ~LocationDelegate() override = default;

    void paintItem(
            QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;
};
